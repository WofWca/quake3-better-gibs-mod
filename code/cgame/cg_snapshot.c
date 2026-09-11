// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_snapshot.c -- things that happen on snapshot transition,
// not necessarily every single rendered frame

#include "cg_local.h"


/*
=====================================================================

KILLCAM SNAPSHOT RECORDING AND PLAYBACK

Every snapshot the live context reads from the engine is copied into a
ring buffer. The killcam context replays them with a time delay: its
CG_ProcessSnapshots reads from the ring instead of trap_GetSnapshot.

=====================================================================
*/

// How many of the most recent snapshots are kept for killcam playback.
// The engine itself only buffers PACKET_BACKUP (32); a useful killcam
// delay needs more history. Raw snapshot_t storage is large (~55 KB
// each): 512 slots is ~27.6 MB of bss and covers
// this times `cg_killcamRecordInterval` milliseconds of history.
// TODO: consider compressing (delta encoding like the engine's) to
// afford a longer history in less memory.
#define KILLCAM_SNAPSHOT_BACKUP	512

// when the preroll gets clamped to the oldest recorded snapshot, keep
// this much slack (ms) so playback doesn't ride the ring's eviction
// edge and abort mid-replay
#define KILLCAM_CLAMP_MARGIN	200

// define KILLCAM_NO_MISSILE_CHASE to compile the missile-chase camera out
#ifndef KILLCAM_NO_MISSILE_CHASE
// a missile explosion within this distance of the victim counts as the
// killing missile when there was no direct hit (splash kills)
#define KILLCAM_MISSILE_SPLASH_DIST	300
// two recorded states of the same entity number are considered the
// same missile if extrapolating the older one lands within this many
// units of the newer one (bounces between snapshots leave some error)
#define KILLCAM_MISSILE_CONTINUITY_DIST	128
// how far back from the obituary time to look for the snapshot in
// which the victim died (ms)
#define KILLCAM_MISSILE_DEATH_SCAN	2000
#endif // KILLCAM_NO_MISSILE_CHASE

static snapshot_t	cg_killcamSnapshots[KILLCAM_SNAPSHOT_BACKUP];
// total snapshots ever recorded; snapshot n (1-based) lives in
// slot (n-1) % KILLCAM_SNAPSHOT_BACKUP until overwritten
static int			cg_killcamRecordedCount;
// the killcam context's read cursor: how many recorded snapshots it has
// consumed. Counterpart of cgs.processedSnapshotNum for the live context.
static int			cg_killcamProcessedNum;
static qboolean		cg_killcamRunning;
static killcamMode_t	cg_killcamMode = KILLCAM_OFF;	// of the current run
static int			cg_killcamCurDelay;		// ms the current run lags behind live time
static int			cg_killcamKillerNum = -1;

// cg_killcamTimescale (slow mo): the replay clock runs slower than
// real time, so the delay grows every frame. cg_killcamDelayFrac keeps
// the sub-millisecond remainder, cg_killcamLastTime is the previous
// frame's serverTime.
static float		cg_killcamDelayFrac;
static int			cg_killcamLastTime;

// scheduled death replay (set when the local player gets killed)
static qboolean		cg_killcamDeathPending;
static int			cg_killcamDeathTime;	// cg.time when the obituary arrived
static int			cg_killcamDeathKiller;
static int			cg_killcamDeathMod;		// means of death from the obituary
// for cg_killcamStartOnAttack: only a fresh attack press (after a
// release) skips the start delay, since the player was likely still
// holding fire when they died
static qboolean		cg_killcamAttackWasUp;
// same, for the jump key (see CG_KillcamJumpPressed)
static qboolean		cg_killcamJumpWasUp;
// server time before which pressing jump doesn't skip the running replay
// (see cg_killcamSkipOnJumpDelay)
static int			cg_killcamSkipAllowedTime;

#ifndef KILLCAM_NO_MISSILE_CHASE
// the missile that scored the kill, for the missile-chase camera
static int			cg_killcamMissileNum = -1;
static int			cg_killcamMissileExplodeTime;
// serverTime of the first recorded snapshot of *this* missile: the
// server reuses entity numbers, so before this time the same number
// may have belonged to a different missile
static int			cg_killcamMissileStartTime;
#endif // KILLCAM_NO_MISSILE_CHASE


static int	cg_killcamLastRecordTime = -99999;
static qboolean CG_KillcamShouldRecord( int serverTime ) {
	int		delta = serverTime - cg_killcamLastRecordTime;

	return (
		delta >= cg_killcamRecordInterval.integer
		// delta < 0 means the level restarted: record and resync
		|| delta < 0
	);
}
// `snap` is the caller's copy of snapshot `num`
static void CG_KillcamRecordSnapshot( int num, const snapshot_t *snap ) {
	qboolean	r;
	snapshot_t	*dest = &cg_killcamSnapshots[
		cg_killcamRecordedCount % KILLCAM_SNAPSHOT_BACKUP
	];
	if ( snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) {
		return;
	}
	if ( !CG_KillcamShouldRecord( snap->serverTime ) ) {
		return;
	}

// `trap_GetSnapshot` might actually be faster than copying the whole struct,
// because a `snapshot_t` struct has space for `MAX_GENTITIES` ents,
// whereas actual snaps only contain a few, so we need to copy less stuff.
#ifndef KILLCAM_COPY_SNAPSHOT
	r = trap_GetSnapshot( num, dest );
	if ( !r ) {
		CG_Printf( S_COLOR_YELLOW "WARNING: expected CG_KillcamRecordSnapshot to get called only when a snapshot exists\n" );
		return;
	}
#else
	*dest = *snap;
#endif
	cg_killcamRecordedCount++;
	cg_killcamLastRecordTime = snap->serverTime;
}


qboolean CG_KillcamRunning( void ) {
	return cg_killcamRunning;
}


/*
==================
CG_KillcamOldestSnapshotTime

serverTime of the oldest snapshot still in the ring, or -1 if none
==================
*/
static int CG_KillcamOldestSnapshotTime( void ) {
	int		oldest;

	if ( cg_killcamRecordedCount == 0 ) {
		return -1;
	}
	oldest = cg_killcamRecordedCount - KILLCAM_SNAPSHOT_BACKUP;
	if ( oldest < 0 ) {
		oldest = 0;
	}
	return cg_killcamSnapshots[oldest % KILLCAM_SNAPSHOT_BACKUP].serverTime;
}


/*
==================
CG_KillcamHasSnapshotFor

qtrue if the ring still holds a snapshot at or before the given time,
i.e. a killcam view of that time can be rendered
==================
*/
qboolean CG_KillcamHasSnapshotFor( int time ) {
	int		oldestTime = CG_KillcamOldestSnapshotTime();

	return oldestTime != -1 && oldestTime <= time;
}


/*
==================
CG_KillcamStart

Resets the killcam context and points its snapshot cursor at the newest
recorded snapshot at or before the given playback time, so the first
killcam frame doesn't fast-forward through (and fire the events of)
older history. CG_ProcessSnapshots does the rest on the next killcam
frame, just like after a fresh connect.
==================
*/
void CG_KillcamStart( int time, killcamMode_t mode ) {
	cgContext_t	*kc = &cg_contexts[CG_CONTEXT_KILLCAM];
	int			oldest;
	int			i;

	memset( kc, 0, sizeof( *kc ) );
	kc->state.clientNum = cg_contexts[CG_CONTEXT_LIVE].state.clientNum;
	// the replayed stream is conceptually a demo: interpolate instead of
	// predicting, never send anything to the server
	kc->state.demoPlayback = qtrue;

	CG_InitLocalEntitiesCtx( CG_CONTEXT_KILLCAM );
	CG_InitMarkPolysCtx( CG_CONTEXT_KILLCAM );
	CG_ClearParticlesCtx( CG_CONTEXT_KILLCAM );

	oldest = cg_killcamRecordedCount - KILLCAM_SNAPSHOT_BACKUP;
	if ( oldest < 0 ) {
		oldest = 0;
	}
	cg_killcamProcessedNum = oldest;
	for ( i = cg_killcamRecordedCount - 1 ; i >= oldest ; i-- ) {
		if ( cg_killcamSnapshots[i % KILLCAM_SNAPSHOT_BACKUP].serverTime <= time ) {
			cg_killcamProcessedNum = i;
			break;
		}
	}

	cg_killcamMode = mode;
#ifndef KILLCAM_NO_MISSILE_CHASE
	cg_killcamMissileNum = -1;
#endif // KILLCAM_NO_MISSILE_CHASE
	CG_KillcamViewReset();
	cg_killcamRunning = qtrue;
}


#ifndef KILLCAM_NO_MISSILE_CHASE
int CG_KillcamMissileNum( void ) {
	return cg_killcamMissileNum;
}


int CG_KillcamMissileExplodeTime( void ) {
	return cg_killcamMissileExplodeTime;
}


int CG_KillcamMissileStartTime( void ) {
	return cg_killcamMissileStartTime;
}


/*
==================
CG_KillcamSnapEntity

The entity with the given number in a recorded snapshot, or NULL
==================
*/
static const entityState_t *CG_KillcamSnapEntity( const snapshot_t *snap, int number ) {
	int		e;

	for ( e = 0 ; e < snap->numEntities ; e++ ) {
		if ( snap->entities[e].number == number ) {
			return &snap->entities[e];
		}
	}
	return NULL;
}

static int CG_MissileWeaponForMod( int mod ) {
	switch ( mod ) {
	case MOD_GRENADE:
	case MOD_GRENADE_SPLASH:
		return WP_GRENADE_LAUNCHER;
	case MOD_ROCKET:
	case MOD_ROCKET_SPLASH:
		return WP_ROCKET_LAUNCHER;
	case MOD_PLASMA:
	case MOD_PLASMA_SPLASH:
		return WP_PLASMAGUN;
	case MOD_BFG:
	case MOD_BFG_SPLASH:
		return WP_BFG;
	default:
		return WP_NONE;
	}
}


/*
==================
CG_KillcamFindMissile

Scans the recorded snapshots around the death for the explosion of the
missile that scored the kill. When a missile explodes, the server turns
it into an ET_GENERAL entity carrying an EV_MISSILE_* event, the
explosion position in pos.trBase, and -- for direct hits -- the hit
player in otherEntityNum (see G_MissileImpact / G_ExplodeMissile).
Direct hits on the victim win; otherwise the explosion closest to the
victim within splash range.

Sets cg_killcamMissileNum / cg_killcamMissileExplodeTime; must be
called after CG_KillcamStart (which resets them).
==================
*/
static void CG_KillcamFindMissile( int victimNum, int mod, int deathTime ) {
	int			i, e;
	int			oldest;
	float		bestDist;
	qboolean	bestDirect;
	int			bestSnapNum;
	int			bestWeapon;
	const int	modMissileWeapon = CG_MissileWeaponForMod( mod );

	if ( !cg_killcamMissile.integer ) {
		return;
	}
	if ( modMissileWeapon == WP_NONE && mod < MOD_NUM_MAX ) {
		// A means of death that we know, and it's not a missile one.
		// (an unknown one might be a non-vanilla missile weapon,
		// see below)
		return;
	}

	oldest = cg_killcamRecordedCount - KILLCAM_SNAPSHOT_BACKUP;
	if ( oldest < 0 ) {
		oldest = 0;
	}

	// The killing damage and the death happen in the same server
	// frame (G_MissileImpact -> G_Damage -> player_die), so the
	// killing explosion and the death first appear in the same
	// snapshot. Find the (newest) snapshot around the obituary time
	// in which the victim went from alive to dead, and look for the
	// explosion only there: explosions in earlier snapshots (e.g. a
	// non-fatal direct hit right before a killing splash) and in
	// later ones (e.g. another player's missile gibbing our corpse)
	// cannot be the killing one.
	bestSnapNum = -1;
	for ( i = cg_killcamRecordedCount - 1 ; i > oldest ; i-- ) {
		const snapshot_t *snap = &cg_killcamSnapshots[i % KILLCAM_SNAPSHOT_BACKUP];
		const snapshot_t *prev = &cg_killcamSnapshots[( i - 1 ) % KILLCAM_SNAPSHOT_BACKUP];

		if ( snap->serverTime > deathTime ) {
			continue;
		}
		if ( snap->serverTime < deathTime - KILLCAM_MISSILE_DEATH_SCAN ) {
			break;
		}
		if ( snap->ps.stats[STAT_HEALTH] <= 0 &&
			prev->ps.stats[STAT_HEALTH] > 0 )
		{
			bestSnapNum = i;
			break;
		}
	}
	if ( bestSnapNum == -1 ) {
		return;
	}

	bestDist = KILLCAM_MISSILE_SPLASH_DIST;
	bestDirect = qfalse;
	bestWeapon = WP_NONE;

	{
		const snapshot_t *snap = &cg_killcamSnapshots[bestSnapNum % KILLCAM_SNAPSHOT_BACKUP];

		for ( e = 0 ; e < snap->numEntities ; e++ ) {
			const entityState_t *es = &snap->entities[e];
			int			event = es->event & ~EV_EVENT_BITS;
			qboolean	direct;
			float		dist;

			if ( es->eType != ET_GENERAL ) {
				continue;
			}
			if ( es->weapon == 0 ) {
				// Probably not a missile but some other general entity.
				continue;
			}
			if ( es->weapon != modMissileWeapon
				// If a mod has non-vanilla missile weapons,
				// we probably still want to chase those missiles,
				// so only require MOD match for weapons that we recognize.
				&& modMissileWeapon != WP_NONE )
			{
				continue;
			}

			if ( event == EV_MISSILE_HIT && es->otherEntityNum == victimNum ) {
				direct = qtrue;
				dist = 0;
			} else if ( event == EV_MISSILE_MISS || event == EV_MISSILE_MISS_METAL ) {
				direct = qfalse;
				dist = Distance( es->pos.trBase, snap->ps.origin );
			} else {
				continue;
			}

			// prefer a direct hit on the victim, then the splash
			// closest to them
			if ( bestDirect ) {
				continue;
			}
			if ( !direct && dist >= bestDist ) {
				continue;
			}

			bestDirect = direct;
			bestDist = dist;
			bestWeapon = es->weapon;
			cg_killcamMissileNum = es->number;
			cg_killcamMissileExplodeTime = snap->serverTime;
		}
	}

	if ( cg_killcamMissileNum < 0 ) {
		return;
	}

	// The server reuses entity numbers, so within the replay window the
	// same number may earlier have belonged to a different missile.
	// Find where *this* missile's history begins by walking backwards
	// from the explosion while the recorded states form one continuous
	// trajectory; the chase only runs from that time on.
	cg_killcamMissileStartTime = cg_killcamMissileExplodeTime;
	{
		const entityState_t	*newer = NULL;
		int					newerTime = 0;

		for ( i = bestSnapNum - 1 ; i >= oldest ; i-- ) {
			const snapshot_t	*snap = &cg_killcamSnapshots[i % KILLCAM_SNAPSHOT_BACKUP];
			const entityState_t	*es = CG_KillcamSnapEntity( snap, cg_killcamMissileNum );

			if ( !es || es->eType != ET_MISSILE || es->weapon != bestWeapon ) {
				break;
			}
			if ( newer ) {
				vec3_t	predicted, actual;

				// this older state, extrapolated forward, must land
				// about where the newer state actually is (grenade
				// bounces between snapshots leave a bounded error)
				BG_EvaluateTrajectory( &es->pos, newerTime, predicted );
				BG_EvaluateTrajectory( &newer->pos, newerTime, actual );
				if ( Distance( predicted, actual ) > KILLCAM_MISSILE_CONTINUITY_DIST ) {
					// a different missile that wore the same number
					break;
				}
			}
			cg_killcamMissileStartTime = snap->serverTime;
			newer = es;
			newerTime = snap->serverTime;
		}
	}

	// don't bother cutting to the missile for a blink of an eye
	// (e.g. a point-blank rocket): keep the killer camera instead
	if ( cg_killcamMissileMinDuration.integer > 0 &&
		cg_killcamMissileExplodeTime - cg_killcamMissileStartTime <
			cg_killcamMissileMinDuration.integer )
	{
		cg_killcamMissileNum = -1;
	}
}
#endif // KILLCAM_NO_MISSILE_CHASE


void CG_KillcamStop( void ) {
	cg_killcamRunning = qfalse;
	cg_killcamMode = KILLCAM_OFF;
}


killcamMode_t CG_KillcamMode( void ) {
	return cg_killcamRunning ? cg_killcamMode : KILLCAM_OFF;
}


int CG_KillcamKillerNum( void ) {
	return cg_killcamKillerNum;
}


/*
==================
CG_KillcamScheduleDeathReplay

Called from the obituary event when the local player gets killed by
another player. The replay itself is started later by CG_KillcamUpdate.
==================
*/
void CG_KillcamScheduleDeathReplay( int killerNum, int mod, int time ) {
	if ( cg_contextNum != CG_CONTEXT_LIVE ) {
		// obituary events re-fired by the replay itself
		return;
	}
	cg_killcamDeathPending = qtrue;
	cg_killcamDeathKiller = killerNum;
	cg_killcamDeathMod = mod;
	cg_killcamAttackWasUp = qfalse;
	cg_killcamJumpWasUp = qfalse;
	cg_killcamDeathTime = time;
}


/*
==================
CG_KillcamJumpPressed

qtrue once per fresh jump press (the player was likely still holding
jump when they died, so only a press after a release counts).

Unlike attack, jump never makes the server respawn us, so it can both
start the replay early and skip it, without a grace period.
==================
*/
static qboolean CG_KillcamJumpPressed( void ) {
	usercmd_t	cmd;

	if ( !trap_GetUserCmd( trap_GetCurrentCmdNumber(), &cmd ) ) {
		return qfalse;
	}
	if ( cmd.upmove <= 0 ) {
		cg_killcamJumpWasUp = qtrue;
		return qfalse;
	}
	if ( !cg_killcamJumpWasUp ) {
		return qfalse;
	}
	// consume it, so holding jump doesn't also skip the replay it started
	cg_killcamJumpWasUp = qfalse;
	return qtrue;
}


/*
==================
CG_KillcamCrouchHeld
==================
*/
static qboolean CG_KillcamCrouchHeld( void ) {
	usercmd_t	cmd;

	return trap_GetUserCmd( trap_GetCurrentCmdNumber(), &cmd ) && cmd.upmove < 0;
}


/*
==================
CG_KillcamTimescaleAt

cg_killcamTimescale only applies within a window around the kill;
outside it the replay runs at real time, easing back over
cg_killcamTimescaleFade so the speed change isn't abrupt. The default
window is wide enough to cover any replay.
==================
*/
static float CG_KillcamTimescaleAt( int replayTime ) {
	int		offset = replayTime - cg_killcamDeathTime;
	int		fade;
	int		past;	// ms past the edge of the window
	float	scale = cg_killcamTimescale.value;

	if ( offset < -cg_killcamTimescaleBefore.integer ) {
		past = -cg_killcamTimescaleBefore.integer - offset;
		fade = cg_killcamTimescaleFadeIn.integer;
	} else if ( offset > cg_killcamTimescaleAfter.integer ) {
		past = offset - cg_killcamTimescaleAfter.integer;
		fade = cg_killcamTimescaleFadeOut.integer;
	} else {
		return scale;
	}

	// fade 0 keeps the hard cut
	if ( past >= fade ) {
		return 1.0f;
	}
	return scale + ( 1.0f - scale ) * (float)past / fade;
}
/*
==================
CG_KillcamAdvanceDelay

Grows (or reduces) the replay's delay behind live time
by the fraction of this frame that the slowed-down replay clock doesn't cover.
==================
*/
static void CG_KillcamAdvanceDelay( int serverTime ) {
	float	grow;
	int		whole;

	grow = cg_killcamDelayFrac
		+ ( serverTime - cg_killcamLastTime )
		* ( 1.0f - CG_KillcamTimescaleAt( serverTime - cg_killcamCurDelay ) );
	whole = (int)grow;
	cg_killcamCurDelay += whole;
	cg_killcamDelayFrac = grow - whole;
	cg_killcamLastTime = serverTime;
}


static int CG_KillcamSkipPreRespawn( int startTime ) {
	int			oldest;
	int			i;
	int			aliveSince = startTime;
	qboolean	seenAlive = qfalse;

	oldest = cg_killcamRecordedCount - KILLCAM_SNAPSHOT_BACKUP;
	if ( oldest < 0 ) {
		oldest = 0;
	}
	for ( i = cg_killcamRecordedCount - 1 ; i >= oldest ; i-- ) {
		const snapshot_t	*snap = &cg_killcamSnapshots[i % KILLCAM_SNAPSHOT_BACKUP];

		if ( snap->serverTime > cg_killcamDeathTime ) {
			continue;
		}
		if ( snap->serverTime <= startTime ) {
			// alive all the way back to the requested start
			break;
		}
		if ( snap->ps.pm_type != PM_DEAD ) {
			seenAlive = qtrue;
			aliveSince = snap->serverTime;
			continue;
		}
		if ( seenAlive ) {
			// dead again: this is an earlier death, so the replay
			// starts at the respawn that follows it
			return aliveSince;
		}
		// still in the dead snapshots of the death being replayed
	}
	return startTime;
}


/*
==================
CG_KillcamUpdate

Runs the killcam state machine once per frame (with the live context
current). Returns how many milliseconds behind live time the killcam
context should be rendered this frame, or 0 to render the live view.
==================
*/
int CG_KillcamUpdate( int serverTime ) {
	// dev/test mode (cg_killcamTest <ms>) takes priority: replay of the
	// own view at a fixed delay
	if ( cg_killcamTest.integer > 0 ) {
		int delay = cg_killcamTest.integer;

		cg_killcamDeathPending = qfalse;
		if ( !cg_killcamRunning && CG_KillcamHasSnapshotFor( serverTime - delay ) ) {
			CG_KillcamStart( serverTime - delay, KILLCAM_TEST );
		}
		if ( cg_killcamRunning && cg_killcamMode == KILLCAM_TEST &&
			CG_KillcamHasSnapshotFor( serverTime - delay ) )
		{
			return delay;
		}
		return 0;
	}
	if ( cg_killcamRunning && cg_killcamMode == KILLCAM_TEST ) {
		CG_KillcamStop();
	}

	if ( !cg_killcam.integer ) {
		cg_killcamDeathPending = qfalse;
		if ( cg_killcamRunning ) {
			CG_KillcamStop();
		}
		return 0;
	}

	// death replay in progress?
	if ( cg_killcamRunning && cg_killcamMode == KILLCAM_KILLER ) {
		int postroll = cg_killcamPostroll.integer;

		CG_KillcamAdvanceDelay( serverTime );
		if ( postroll < 0 ) {
			postroll = 0;
		}
		if (
			// replay finished
			serverTime - cg_killcamCurDelay > cg_killcamDeathTime + postroll
			// the player respawned (e.g. clicked): hand the view back
			|| cg.predictedPlayerState.stats[STAT_HEALTH] > 0
			// the player pressed jump: skip the rest of the replay
			|| ( CG_KillcamJumpPressed() && serverTime >= cg_killcamSkipAllowedTime )
			// recording outran the playback; can't render this frame
			|| !CG_KillcamHasSnapshotFor( serverTime - cg_killcamCurDelay ) )
		{
			CG_KillcamStop();
			return 0;
		}
		return cg_killcamCurDelay;
	}

	// scheduled death replay waiting to start?
	if ( cg_killcamDeathPending ) {
		int			replayStartTime;
		int			oldestTime;
		qboolean	startNow = qfalse;
		int			preroll = cg_killcamPreroll.integer;
		int			startDelay = cg_killcamStartDelay.integer;

		if ( preroll < 0 ) {
			preroll = 0;
		}
		if ( startDelay < 0 ) {
			startDelay = 0;
		}

		// let the death register on screen before switching views;
		// holding crouch holds the auto-start off for longer
		if ( serverTime < cg_killcamDeathTime + startDelay || CG_KillcamCrouchHeld() ) {
			// a fresh attack press (click) skips the wait, once the
			// grace period against accidental clicks has passed.
			// Note that the server ignores attack presses of dead
			// players for ~1.7 s after the death, so an early click
			// doesn't respawn us; later ones do (and end the
			// killcam), as usual.
			if ( cg_killcamStartOnClickDelay.integer >= 0 &&
				serverTime >= cg_killcamDeathTime + cg_killcamStartOnClickDelay.integer )
			{
				usercmd_t	cmd;

				if ( trap_GetUserCmd( trap_GetCurrentCmdNumber(), &cmd ) ) {
					if ( !( cmd.buttons & BUTTON_ATTACK ) ) {
						cg_killcamAttackWasUp = qtrue;
					} else if ( cg_killcamAttackWasUp ) {
						startNow = qtrue;
					}
				}
			}
			// jump doesn't respawn us, so it's always allowed to
			// start the replay early, after its own grace period
			if ( cg_killcamStartOnJumpDelay.integer >= 0 &&
				serverTime >= cg_killcamDeathTime + cg_killcamStartOnJumpDelay.integer &&
				CG_KillcamJumpPressed() )
			{
				startNow = qtrue;
			}
			if ( !startNow ) {
				return 0;
			}
		}
		cg_killcamDeathPending = qfalse;

		cg_killcamSkipAllowedTime = serverTime;
		if ( !startNow ) {
			// the killcam started automatically, so a jump press around
			// the moment it pops up probably isn't meant to skip it
			cg_killcamSkipAllowedTime += cg_killcamSkipOnJumpDelay.integer;
		}

		// skip if the player already respawned or the game is ending
		if ( cg.predictedPlayerState.stats[STAT_HEALTH] > 0 || cg.intermissionStarted ) {
			return 0;
		}

		// skip if not even the death itself is in the recorded history
		// (e.g. very high snaps rate combined with a long start delay)
		oldestTime = CG_KillcamOldestSnapshotTime();
		if ( oldestTime == -1 || oldestTime > cg_killcamDeathTime ) {
			return 0;
		}

		// if the ring no longer holds the full preroll, start at the
		// oldest snapshot we still have (with some slack so playback
		// doesn't ride the ring's eviction edge)
		replayStartTime = cg_killcamDeathTime - preroll;
		if ( replayStartTime < oldestTime + KILLCAM_CLAMP_MARGIN ) {
			replayStartTime = oldestTime + KILLCAM_CLAMP_MARGIN;
			if ( replayStartTime > cg_killcamDeathTime ) {
				replayStartTime = cg_killcamDeathTime;
			}
		}

		if ( cg_killcamSkipPreRespawn.integer ) {
			replayStartTime = CG_KillcamSkipPreRespawn( replayStartTime );
		}

		cg_killcamCurDelay = serverTime - replayStartTime;
		cg_killcamDelayFrac = 0;
		cg_killcamLastTime = serverTime;
		cg_killcamKillerNum = cg_killcamDeathKiller;
		CG_KillcamStart( replayStartTime, KILLCAM_KILLER );
#ifndef KILLCAM_NO_MISSILE_CHASE
		// after Start (it resets the missile): find the killing missile
		// for the missile-chase camera
		CG_KillcamFindMissile( cg.snap->ps.clientNum, cg_killcamDeathMod,
			cg_killcamDeathTime );
#endif // KILLCAM_NO_MISSILE_CHASE
		return cg_killcamCurDelay;
	}

	return 0;
}


/*
==================
CG_KillcamReadNextSnapshot

The killcam context's counterpart of CG_ReadNextSnapshot: reads from the
ring buffer instead of the client system.
==================
*/
static snapshot_t *CG_KillcamReadNextSnapshot( void ) {
	snapshot_t	*dest;
	int			oldestAvailable;

	// if playback fell behind the recording, skip the overwritten ones
	oldestAvailable = cg_killcamRecordedCount - KILLCAM_SNAPSHOT_BACKUP;
	if ( cg_killcamProcessedNum < oldestAvailable ) {
		cg_killcamProcessedNum = oldestAvailable;
	}

	if ( cg_killcamProcessedNum >= cg_killcamRecordedCount ) {
		// nothing left to read
		return NULL;
	}

	// decide which of the two slots to load it into
	if ( cg.snap == &cg.activeSnapshots[0] ) {
		dest = &cg.activeSnapshots[1];
	} else {
		dest = &cg.activeSnapshots[0];
	}

	*dest = cg_killcamSnapshots[cg_killcamProcessedNum % KILLCAM_SNAPSHOT_BACKUP];
	cg_killcamProcessedNum++;
	return dest;
}


/*
==================
CG_ResetEntity
==================
*/
static void CG_ResetEntity( centity_t *cent ) {
	// if the previous snapshot this entity was updated in is at least
	// an event window back in time then we can reset the previous event
	if ( cent->snapShotTime < cg.time - EVENT_VALID_MSEC ) {
		cent->previousEvent = 0;
	}

	cent->trailTime = cg.snap->serverTime;

	VectorCopy (cent->currentState.origin, cent->lerpOrigin);
	VectorCopy (cent->currentState.angles, cent->lerpAngles);
	if ( cent->currentState.eType == ET_PLAYER ) {
		CG_ResetPlayerEntity( cent );
	}
}

/*
===============
CG_TransitionEntity

cent->nextState is moved to cent->currentState and events are fired
===============
*/
static void CG_TransitionEntity( centity_t *cent ) {
	cent->currentState = cent->nextState;
	cent->currentValid = qtrue;

	// reset if the entity wasn't in the last frame or was teleported
	if ( !cent->interpolate ) {
		CG_ResetEntity( cent );
	}

	// clear the next state.  if will be set by the next CG_SetNextSnap
	cent->interpolate = qfalse;

	// check for events
	CG_CheckEvents( cent );

	if ( !cg_oldGibs.integer && cg_gibsStopPlayerSounds.integer & 0x1 ) {
		CG_CheckMuteSoundsIfGibbed( cent );
	}
}


/*
==================
CG_SetInitialSnapshot

This will only happen on the very first snapshot, or
on tourney restarts.  All other times will use 
CG_TransitionSnapshot instead.

FIXME: Also called by map_restart?
==================
*/
void CG_SetInitialSnapshot( snapshot_t *snap ) {
	int				i;
	centity_t		*cent;
	entityState_t	*state;

	cg.snap = snap;

	BG_PlayerStateToEntityState( &snap->ps, &cg_entities[ snap->ps.clientNum ].currentState, qfalse );

	// sort out solid entities
	CG_BuildSolidList();

	CG_ExecuteNewServerCommands( snap->serverCommandSequence );

	// set our local weapon selection pointer to
	// what the server has indicated the current weapon is
	CG_Respawn();

	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		state = &cg.snap->entities[ i ];
		cent = &cg_entities[ state->number ];

		memcpy(&cent->currentState, state, sizeof(entityState_t));
		//cent->currentState = *state;
		cent->interpolate = qfalse;
		cent->currentValid = qtrue;

		CG_ResetEntity( cent );

		// check for events
		CG_CheckEvents( cent );
	}
}


/*
===================
CG_TransitionSnapshot

The transition point from snap to nextSnap has passed
===================
*/
static void CG_TransitionSnapshot( void ) {
	centity_t			*cent;
	snapshot_t			*oldFrame;
	int					i;

	if ( !cg.snap ) {
		CG_Error( "CG_TransitionSnapshot: NULL cg.snap" );
	}
	if ( !cg.nextSnap ) {
		CG_Error( "CG_TransitionSnapshot: NULL cg.nextSnap" );
	}

	// execute any server string commands before transitioning entities
	CG_ExecuteNewServerCommands( cg.nextSnap->serverCommandSequence );

	// if we had a map_restart, set everthing with initial
	if ( !cg.snap ) {
		return;
	}

	// clear the currentValid flag for all entities in the existing snapshot
	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		cent = &cg_entities[ cg.snap->entities[ i ].number ];
		cent->currentValid = qfalse;
	}

	// move nextSnap to snap and do the transitions
	oldFrame = cg.snap;
	cg.snap = cg.nextSnap;

	// A bit of a hack to fix the respawn sound (`teleInSound`) not playing
	// when the killcam is interrupted by a respawn.
	// `CG_KillcamUpdate`, which is responsible for ending the killcam,
	// only ends it on the next frame after respawn.
	//
	// TODO a proper fix would be to first process snapshots
	// and decide to draw either the live or the killcam frame
	// based on whether we've respawned.
	if ( cg_contextNum == CG_CONTEXT_LIVE
		&& cg.snap->ps.stats[STAT_HEALTH] > 0 )
	{
		cg_soundMuted = qfalse;
	}

	BG_PlayerStateToEntityState( &cg.snap->ps, &cg_entities[ cg.snap->ps.clientNum ].currentState, qfalse );
	cg_entities[ cg.snap->ps.clientNum ].interpolate = qfalse;
	if ( !cg_oldGibs.integer && cg_gibsStopPlayerSounds.integer & 0x2 ) {
		CG_CheckMuteSoundsIfGibbed( &cg_entities[ cg.snap->ps.clientNum ] );
	}

	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		cent = &cg_entities[ cg.snap->entities[ i ].number ];
		CG_TransitionEntity( cent );

		// remember time of snapshot this entity was last updated in
		cent->snapShotTime = cg.snap->serverTime;
	}

	cg.nextSnap = NULL;

	// check for playerstate transition events
	if ( oldFrame ) {
		playerState_t	*ops, *ps;

		ops = &oldFrame->ps;
		ps = &cg.snap->ps;
		// teleporting checks are irrespective of prediction
		if ( ( ps->eFlags ^ ops->eFlags ) & EF_TELEPORT_BIT ) {
			cg.thisFrameTeleport = qtrue;	// will be cleared by prediction code
		}

		// if we are not doing client side movement prediction for any
		// reason, then the client events and view changes will be issued now
		if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW)
			|| cg_nopredict.integer || cgs.synchronousClients ) {
			CG_TransitionPlayerState( ps, ops );
		}
	}
}


/*
===================
CG_TransitionNoLerpEntities

Some events do not need interpolation and can do their thing
as soon as they appear in `cg.nextSnap`,
without waiting to transition from `cg.nextSnap` to `cg.snap`
(`CG_TransitionSnapshot`).
===================
*/
static void CG_TransitionNoLerpEntities( const snapshot_t *snap ) {
	if ( cg_gibsNoLerpDelay.integer && !cg_oldGibs.integer ) {
		int num;

		for ( num = 0 ; num < snap->numEntities ; num++ ) {
			const entityState_t *es = &snap->entities[num];
			centity_t *cent = &cg_entities[ es->number ];
			// We have roughly the same calculations in `CG_GibPlayer2`.
			int targNum =
				es->eType == ET_EVENTS + EV_GIB_PLAYER &&
				cgs.g_gibsNewEvGibPlayerProtocol & 0x04
					? es->otherEntityNum
					: es->number;
			centity_t *targCent = targNum == cg.snap->ps.clientNum
				? &cg.predictedPlayerEntity
				: &cg_entities[ targNum ];
			vec3_t posBase;
			int nextStateEvent;

			if ( es->eType != ET_EVENTS + EV_GIB_PLAYER &&
				// For vanilla servers.
				!(
					( es->event & ~EV_EVENT_BITS ) == EV_GIB_PLAYER &&
					cg_gibsNoLerpDelay.integer & 0x02
				) )
			{
				continue;
			}

			if ( cg_debugGibs.integer & 0x08 ) {
				CG_Printf("Running gib event without lerp delay (cg_gibsNoLerpDelay). ent %i targ %i t %i server time %i\n",
					es->number, targNum, cg.time, snap->serverTime );
			}

			// TODO fix: with vanilla servers (old protocol)
			// this will basically teleport the player
			// to their position on `nextSnap`, without interpolation.
			// So it will look like gibs originated from a position
			// different from the player body's position.
			// Noticeable with quad shotgun and low `snaps` value.
			//
			// Probably can be fixed by only running `CG_GibPlayer`
			// and not touching `cent->currentState`
			// (`CG_TransitionEntity()` does it).

			// This will run `CG_GibPlayer()`.
			CG_TransitionEntity( cent );
			// Copied from the other occurrence of `CG_TransitionEntity`.
			// Otherwise it seems that `CG_GibPlayer()` runs twice.
			cent->snapShotTime = cg.snap->serverTime;

			// We've run the "gib" event for this entity ahead of time,
			// so we also need to make it invisible ahead of time,
			// otherwise both gibs and the player body will be visible
			// at the same time for a few frames, until the next snap.
			//
			// TODO fix: for `predictedPlayerEntity` this doesn't work,
			// because that is re-created each frame,
			// with `BG_PlayerStateToEntityState`.
			// This matters e.g. during demo playback
			// with `cg.renderingThirdPerson`,
			// where the player state is interpolated,
			// see `CG_InterpolatePlayerState`.
			if ( !( cg_gibsNoLerpDelay.integer & 0x4 ) ) {
				targCent->currentState.eType = targCent->nextState.eType;
			}
		}
	}
}

/*
===================
CG_SetNextSnap

A new snapshot has just been read in from the client system.
===================
*/
static void CG_SetNextSnap( snapshot_t *snap ) {
	int					num;
	int					esNum;
	entityState_t		*es;
	centity_t			*cent;

	cg.nextSnap = snap;

	BG_PlayerStateToEntityState( &snap->ps, &cg_entities[ snap->ps.clientNum ].nextState, qfalse );
	cg_entities[ cg.snap->ps.clientNum ].interpolate = qtrue;

	// check for extrapolation errors
	for ( num = 0 ; num < snap->numEntities ; num++ ) {
		es = &snap->entities[num];
		cent = &cg_entities[ es->number ];

		memcpy(&cent->nextState, es, sizeof(entityState_t));
		//cent->nextState = *es;

		if ( cgs.ospEnc && ( esNum = cent->nextState.number ) <= MAX_CLIENTS-1 ) {
			cent->nextState.pos.trBase[0] += (677 - 7 * esNum);
			cent->nextState.pos.trBase[1] += (411 - 12 * esNum);
			cent->nextState.pos.trBase[2] += (243 - 2 * esNum);
		}

		// if this frame is a teleport, or the entity wasn't in the
		// previous frame, don't interpolate
		if ( !cent->currentValid || ( ( cent->currentState.eFlags ^ es->eFlags ) & EF_TELEPORT_BIT )  ) {
			cent->interpolate = qfalse;
		} else {
			cent->interpolate = qtrue;
		}
	}

	// if the next frame is a teleport for the playerstate, we
	// can't interpolate during demos
	if ( cg.snap && ( ( snap->ps.eFlags ^ cg.snap->ps.eFlags ) & EF_TELEPORT_BIT ) ) {
		cg.nextFrameTeleport = qtrue;
	} else {
		cg.nextFrameTeleport = qfalse;
	}

	// if changing follow mode, don't interpolate
	if ( cg.nextSnap->ps.clientNum != cg.snap->ps.clientNum ) {
		cg.nextFrameTeleport = qtrue;
	}

	// if changing server restarts, don't interpolate
	if ( ( cg.nextSnap->snapFlags ^ cg.snap->snapFlags ) & SNAPFLAG_SERVERCOUNT ) {
		cg.nextFrameTeleport = qtrue;
	}

	// sort out solid entities
	CG_BuildSolidList();

	CG_TransitionNoLerpEntities( snap );
}


/*
========================
CG_ReadNextSnapshot

This is the only place new snapshots are requested
This may increment cgs.processedSnapshotNum multiple
times if the client system fails to return a
valid snapshot.
========================
*/
static snapshot_t *CG_ReadNextSnapshot( void ) {
	qboolean	r;
	snapshot_t	*dest;

	if ( cg_contextNum == CG_CONTEXT_KILLCAM ) {
		return CG_KillcamReadNextSnapshot();
	}

	if ( cg.latestSnapshotNum > cgs.processedSnapshotNum + 1000 ) {
		CG_Printf( "WARNING: CG_ReadNextSnapshot: way out of range, %i > %i\n", 
			cg.latestSnapshotNum, cgs.processedSnapshotNum );
	}

	while ( cgs.processedSnapshotNum < cg.latestSnapshotNum ) {
		// decide which of the two slots to load it into
		if ( cg.snap == &cg.activeSnapshots[0] ) {
			dest = &cg.activeSnapshots[1];
		} else {
			dest = &cg.activeSnapshots[0];
		}

		// try to read the snapshot from the client system
		cgs.processedSnapshotNum++;
		r = trap_GetSnapshot( cgs.processedSnapshotNum, dest );

		// FIXME: why would trap_GetSnapshot return a snapshot with the same server time
		if ( cg.snap && r && dest->serverTime == cg.snap->serverTime ) {
			//continue;
		}

		// if it succeeded, return
		if ( r ) {
			CG_AddLagometerSnapshotInfo( dest );
			CG_KillcamRecordSnapshot( cgs.processedSnapshotNum, dest );
			return dest;
		}

		// a GetSnapshot will return failure if the snapshot
		// never arrived, or  is so old that its entities
		// have been shoved off the end of the circular
		// buffer in the client system.

		// record as a dropped packet
		if ( cg.snap ) {
			CG_AddLagometerSnapshotInfo( NULL );
		}

		// If there are additional snapshots, continue trying to
		// read them.
	}

	// nothing left to read
	return NULL;
}


/*
============
CG_ProcessSnapshots

We are trying to set up a renderable view, so determine
what the simulated time is, and try to get snapshots
both before and after that time if available.

If we don't have a valid cg.snap after exiting this function,
then a 3D game view cannot be rendered.  This should only happen
right after the initial connection.  After cg.snap has been valid
once, it will never turn invalid.

Even if cg.snap is valid, cg.nextSnap may not be, if the snapshot
hasn't arrived yet (it becomes an extrapolating situation instead
of an interpolating one)

============
*/
void CG_ProcessSnapshots( void ) {
	snapshot_t		*snap;
	int				n;

	// see what the latest snapshot the client system has is
	if ( cg_contextNum == CG_CONTEXT_KILLCAM ) {
		// the killcam context reads recorded snapshots instead
		n = cg_killcamRecordedCount;
		cg.latestSnapshotTime = cg_killcamRecordedCount > 0
			? cg_killcamSnapshots[( cg_killcamRecordedCount - 1 ) % KILLCAM_SNAPSHOT_BACKUP].serverTime
			: 0;
	} else {
		trap_GetCurrentSnapshotNumber( &n, &cg.latestSnapshotTime );
	}
	if ( n != cg.latestSnapshotNum ) {
		if ( n < cg.latestSnapshotNum ) {
			// this should never happen
			CG_Error( "CG_ProcessSnapshots: n < cg.latestSnapshotNum" );
		}
		cg.latestSnapshotNum = n;
	}

	// If we have yet to receive a snapshot, check for it.
	// Once we have gotten the first snapshot, cg.snap will
	// always have valid data for the rest of the game
	while ( !cg.snap ) {
		snap = CG_ReadNextSnapshot();
		if ( !snap ) {
			// we can't continue until we get a snapshot
			return;
		}

		// set our weapon selection to what
		// the playerstate is currently using
		if ( !( snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
			CG_SetInitialSnapshot( snap );
		}
	}

	// loop until we either have a valid nextSnap with a serverTime
	// greater than cg.time to interpolate towards, or we run
	// out of available snapshots
	do {
		// if we don't have a nextframe, try and read a new one in
		if ( !cg.nextSnap ) {
			snap = CG_ReadNextSnapshot();

			// if we still don't have a nextframe, we will just have to
			// extrapolate
			if ( !snap ) {
				break;
			}

			CG_SetNextSnap( snap );

			// if time went backwards, we have a level restart
			if ( cg.nextSnap->serverTime < cg.snap->serverTime ) {
				CG_Error( "CG_ProcessSnapshots: Server time went backwards" );
			}
		}

		// if our time is < nextFrame's, we have a nice interpolating state
		if ( cg.time >= cg.snap->serverTime && cg.time < cg.nextSnap->serverTime ) {
			break;
		}

		// we have passed the transition from nextFrame to frame
		CG_TransitionSnapshot();
	} while ( 1 );

	// assert our valid conditions upon exiting
	if ( cg.snap == NULL ) {
		CG_Error( "CG_ProcessSnapshots: cg.snap == NULL" );
	}
	if ( cg.time < cg.snap->serverTime ) {
		// this can happen right after a vid_restart
		cg.time = cg.snap->serverTime;
	}
	if ( cg.nextSnap != NULL && cg.nextSnap->serverTime <= cg.time ) {
		CG_Error( "CG_ProcessSnapshots: cg.nextSnap->serverTime <= cg.time" );
	}
}
