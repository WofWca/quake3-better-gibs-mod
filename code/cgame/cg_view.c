// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering
#include "cg_local.h"


/*
=============================================================================

  MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like 
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and supress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

If a gun is being tested, the "gun_x", "gun_y", and "gun_z" variables will let
you adjust the positioning.

Note that none of the model testing features update while the game is paused, so
it may be convenient to test with deathmatch set to 1 so that bringing down the
console doesn't pause the game.

=============================================================================
*/

/*
=================
CG_TestModel_f

Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f (void) {
	vec3_t		angles;

	memset( &cg.testModelEntity, 0, sizeof(cg.testModelEntity) );
	if ( trap_Argc() < 2 ) {
		return;
	}

	Q_strncpyz (cg.testModelName, CG_Argv( 1 ), MAX_QPATH );
	cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );

	if ( trap_Argc() == 3 ) {
		cg.testModelEntity.backlerp = atof( CG_Argv( 2 ) );
		cg.testModelEntity.frame = 1;
		cg.testModelEntity.oldframe = 0;
	}

	if ( !cg.testModelEntity.hModel ) {
		CG_Printf( "Can't register model '%s'.\n", cg.testModelName );
		return;
	}

	VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testModelEntity.origin );

	angles[PITCH] = 0;
	angles[YAW] = 180 + cg.refdefViewAngles[1];
	angles[ROLL] = 0;

	AnglesToAxis( angles, cg.testModelEntity.axis );
	cg.testGun = qfalse;
}

/*
=================
CG_TestGun_f

Replaces the current view weapon with the given model
=================
*/
void CG_TestGun_f (void) {
	CG_TestModel_f();
	cg.testGun = qtrue;
	cg.testModelEntity.renderfx = RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON;
}


void CG_TestModelNextFrame_f (void) {
	cg.testModelEntity.frame++;
	CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelPrevFrame_f (void) {
	cg.testModelEntity.frame--;
	if ( cg.testModelEntity.frame < 0 ) {
		cg.testModelEntity.frame = 0;
	}
	CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelNextSkin_f (void) {
	cg.testModelEntity.skinNum++;
	CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_TestModelPrevSkin_f (void) {
	cg.testModelEntity.skinNum--;
	if ( cg.testModelEntity.skinNum < 0 ) {
		cg.testModelEntity.skinNum = 0;
	}
	CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

static void CG_AddTestModel (void) {
	int		i;

	// re-register the model, because the level may have changed
	cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );
	if (! cg.testModelEntity.hModel ) {
		CG_Printf ("Can't register model\n");
		return;
	}

	// if testing a gun, set the origin relative to the view origin
	if ( cg.testGun ) {
		VectorCopy( cg.refdef.vieworg, cg.testModelEntity.origin );
		VectorCopy( cg.refdef.viewaxis[0], cg.testModelEntity.axis[0] );
		VectorCopy( cg.refdef.viewaxis[1], cg.testModelEntity.axis[1] );
		VectorCopy( cg.refdef.viewaxis[2], cg.testModelEntity.axis[2] );

		// allow the position to be adjusted
		for (i=0 ; i<3 ; i++) {
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[0][i] * cg_gun_x.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[1][i] * cg_gun_y.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[2][i] * cg_gun_z.value;
		}
	}

	trap_R_AddRefEntityToScene( &cg.testModelEntity );
}



//============================================================================


/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect (void) {
	int		size;

	// the intermission should allways be full screen
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		size = 100;
	} else {
		// bound normal viewsize
		if (cg_viewsize.integer < 30) {
			trap_Cvar_Set ("cg_viewsize","30");
			size = 30;
		} else if (cg_viewsize.integer > 100) {
			trap_Cvar_Set ("cg_viewsize","100");
			size = 100;
		} else {
			size = cg_viewsize.integer;
		}

	}
	cg.refdef.width = cgs.glconfig.vidWidth*size/100;
	cg.refdef.width &= ~1;

	cg.refdef.height = cgs.glconfig.vidHeight*size/100;
	cg.refdef.height &= ~1;

	cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width)/2;
	cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height)/2;
}

//==============================================================================


/*
===============
CG_OffsetThirdPersonView

===============
*/
#define	FOCUS_DISTANCE	512
static void CG_OffsetThirdPersonView( void ) {
	vec3_t		forward, right, up;
	vec3_t		view;
	vec3_t		focusAngles;
	trace_t		trace;
	static vec3_t	mins = { -4, -4, -4 };
	static vec3_t	maxs = { 4, 4, 4 };
	vec3_t		focusPoint;
	float		focusDist;
	float		forwardScale, sideScale;

	cg.refdef.vieworg[2] += cg.predictedPlayerState.viewheight;

	VectorCopy( cg.refdefViewAngles, focusAngles );

	// if dead, look at killer
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		focusAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
		cg.refdefViewAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
	}

	if ( focusAngles[PITCH] > 45 ) {
		focusAngles[PITCH] = 45;		// don't go too far overhead
	}
	AngleVectors( focusAngles, forward, NULL, NULL );

	VectorMA( cg.refdef.vieworg, FOCUS_DISTANCE, forward, focusPoint );

	VectorCopy( cg.refdef.vieworg, view );

	view[2] += 8;

	cg.refdefViewAngles[PITCH] *= 0.5;

	AngleVectors( cg.refdefViewAngles, forward, right, up );

	forwardScale = cos( cg_thirdPersonAngle.value / 180 * M_PI );
	sideScale = sin( cg_thirdPersonAngle.value / 180 * M_PI );
	VectorMA( view, -cg_thirdPersonRange.value * forwardScale, forward, view );
	VectorMA( view, -cg_thirdPersonRange.value * sideScale, right, view );

	// trace a ray from the origin to the viewpoint to make sure the view isn't
	// in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything

	if (!cg_cameraMode.integer) {
		CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );

		if ( trace.fraction != 1.0 ) {
			VectorCopy( trace.endpos, view );
			view[2] += (1.0 - trace.fraction) * 32;
			// try another trace to this position, because a tunnel may have the ceiling
			// close enough that this is poking out

			CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );
			VectorCopy( trace.endpos, view );
		}
	}


	VectorCopy( view, cg.refdef.vieworg );

	// select pitch to look at focus point from vieword
	VectorSubtract( focusPoint, cg.refdef.vieworg, focusPoint );
	focusDist = sqrt( focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1] );
	if ( focusDist < 1 ) {
		focusDist = 1;	// should never happen
	}
	cg.refdefViewAngles[PITCH] = -180 / M_PI * atan2( focusPoint[2], focusDist );
	cg.refdefViewAngles[YAW] -= cg_thirdPersonAngle.value;
}


// this causes a compiler bug on mac MrC compiler
static void CG_StepOffset( void ) {
	int		timeDelta;
	
	// smooth out stair climbing
	timeDelta = cg.time - cg.stepTime;
	if ( timeDelta < STEP_TIME ) {
		cg.refdef.vieworg[2] -= cg.stepChange 
			* (STEP_TIME - timeDelta) / STEP_TIME;
	}
}

/*
===============
CG_OffsetFirstPersonView

===============
*/
static void CG_OffsetFirstPersonView( void ) {
	float			*origin;
	float			*angles;
	float			bob;
	float			ratio;
	float			delta;
	float			speed;
	float			f;
	vec3_t			predictedVelocity;
	int				timeDelta;
	
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		return;
	}

	origin = cg.refdef.vieworg;
	angles = cg.refdefViewAngles;

	// if dead, fix the angle and don't add any kick
	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
		angles[ROLL] = 40;
		angles[PITCH] = -15;
		angles[YAW] = cg.snap->ps.stats[STAT_DEAD_YAW];
		origin[2] += cg.predictedPlayerState.viewheight;
		return;
	}

	// add angles based on weapon kick
	VectorAdd (angles, cg.kick_angles, angles);

	// add angles based on damage kick
	if ( cg.damageTime ) {
		ratio = cg.time - cg.damageTime;
		if ( ratio < DAMAGE_DEFLECT_TIME ) {
			ratio /= DAMAGE_DEFLECT_TIME;
			angles[PITCH] += ratio * cg.v_dmg_pitch;
			angles[ROLL] += ratio * cg.v_dmg_roll;
		} else {
			ratio = 1.0 - ( ratio - DAMAGE_DEFLECT_TIME ) / DAMAGE_RETURN_TIME;
			if ( ratio > 0 ) {
				angles[PITCH] += ratio * cg.v_dmg_pitch;
				angles[ROLL] += ratio * cg.v_dmg_roll;
			}
		}
	}

	// add pitch based on fall kick
#if 0
	ratio = ( cg.time - cg.landTime) / FALL_TIME;
	if (ratio < 0)
		ratio = 0;
	angles[PITCH] += ratio * cg.fall_value;
#endif

	// add angles based on velocity
	VectorCopy( cg.predictedPlayerState.velocity, predictedVelocity );

	delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[0]);
	angles[PITCH] += delta * cg_runpitch.value;
	
	delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[1]);
	angles[ROLL] -= delta * cg_runroll.value;

	// add angles based on bob

	// make sure the bob is visible even at low speeds
	speed = cg.xyspeed > 200 ? cg.xyspeed : 200;

	delta = cg.bobfracsin * cg_bobpitch.value * speed;
	if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
		delta *= 3;		// crouching
	angles[PITCH] += delta;
	delta = cg.bobfracsin * cg_bobroll.value * speed;
	if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
		delta *= 3;		// crouching accentuates roll
	if (cg.bobcycle & 1)
		delta = -delta;
	angles[ROLL] += delta;

//===================================

	// add view height
	origin[2] += cg.predictedPlayerState.viewheight;

	// smooth out duck height changes
	timeDelta = cg.time - cg.duckTime;
	if ( timeDelta < DUCK_TIME) {
		cg.refdef.vieworg[2] -= cg.duckChange 
			* (DUCK_TIME - timeDelta) / DUCK_TIME;
	}

	// add bob height
	bob = cg.bobfracsin * cg.xyspeed * cg_bobup.value;
	if (bob > 6) {
		bob = 6;
	}

	origin[2] += bob;


	// add fall height
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		f = delta / LAND_DEFLECT_TIME;
		cg.refdef.vieworg[2] += cg.landChange * f;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		delta -= LAND_DEFLECT_TIME;
		f = 1.0 - ( delta / LAND_RETURN_TIME );
		cg.refdef.vieworg[2] += cg.landChange * f;
	}

	// add step offset
	CG_StepOffset();

	// add kick offset

	VectorAdd (origin, cg.kick_origin, origin);

	// pivot the eye based on a neck length
#if 0
	{
#define	NECK_LENGTH		8
	vec3_t			forward, up;
 
	cg.refdef.vieworg[2] -= NECK_LENGTH;
	AngleVectors( cg.refdefViewAngles, forward, NULL, up );
	VectorMA( cg.refdef.vieworg, 3, forward, cg.refdef.vieworg );
	VectorMA( cg.refdef.vieworg, NECK_LENGTH, up, cg.refdef.vieworg );
	}
#endif
}

//======================================================================

void CG_ZoomDown_f( void ) { 
	if ( cg.zoomed ) {
		return;
	}
	cg.zoomed = qtrue;
	cg.zoomTime = cg.time;
}

void CG_ZoomUp_f( void ) { 
	if ( !cg.zoomed ) {
		return;
	}
	cg.zoomed = qfalse;
	cg.zoomTime = cg.time;
}


/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

static int CG_CalcFov( void ) {
	float	x;
	//float	phase;
	float	v;
	int		contents;
	float	fov_x, fov_y;
	float	zoomFov;
	float	f;
	int		inwater;

	cgs.fov = cg_fov.value;
	if ( cgs.fov < 1.0 )
		cgs.fov = 1.0;
	else if ( cgs.fov > 160.0 )
		cgs.fov = 160.0;

	cgs.zoomFov = cg_zoomFov.value;
	if ( cgs.zoomFov < 1.0 )
		cgs.zoomFov = 1.0;
	else if ( cgs.zoomFov > 160.0 )
		cgs.zoomFov = 160.0;

	if ( cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		// if in intermission, use a fixed value
		fov_x = 90;
	} else {
		// user selectable
		fov_x = cgs.fov;

		// account for zooms
		zoomFov = cgs.zoomFov;

		if ( cg.zoomed ) {
			f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
			if ( f > 1.0 ) {
				fov_x = zoomFov;
			} else {
				fov_x = fov_x + f * ( zoomFov - fov_x );
			}
		} else {
			f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
			if ( f > 1.0 ) {
				//fov_x = fov_x;
			} else {
				fov_x = zoomFov + f * ( fov_x - zoomFov );
			}
		}
	}

	if ( cg_fovAdjust.integer ) {
		// Based on LordHavoc's code for Darkplaces
		// http://www.quakeworld.nu/forum/topic/53/what-does-your-qw-look-like/page/30
		const float baseAspect = 0.75f; // 3/4
		const float aspect = (float)cg.refdef.width/(float)cg.refdef.height;
		const float desiredFov = fov_x;

		fov_x = atan2( tan( desiredFov * M_PI / 360.0f ) * baseAspect * aspect, 1 ) * 360.0f / M_PI;
	}

	x = cg.refdef.width / tan( fov_x / 360 * M_PI );
	fov_y = atan2( cg.refdef.height, x );
	fov_y = fov_y * 360 / M_PI;

	// warp if underwater
	contents = CG_PointContents( cg.refdef.vieworg, -1 );
	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ){
		//phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
		//v = WAVE_AMPLITUDE * sin( phase );
		v = WAVE_AMPLITUDE * sin( (cg.time % 16419587) / 397.87735f ); // result is very close to original
		fov_x += v;
		fov_y -= v;
		inwater = qtrue;
	}
	else {
		inwater = qfalse;
	}


	// set it
	cg.refdef.fov_x = fov_x;
	cg.refdef.fov_y = fov_y;

	if ( !cg.zoomed ) {
		cg.zoomSensitivity = 1;
	} else {
		cg.zoomSensitivity = cg.refdef.fov_y / 75.0;
	}

	return inwater;
}



/*
===============
CG_DamageBlendBlob

===============
*/
static void CG_DamageBlendBlob( void ) {
	int			t;
	int			maxTime;
	refEntity_t		ent;

	if (!cg_blood.integer) {
		return;
	}

	if ( !cg.damageValue ) {
		return;
	}

	//if (cg.cameraMode) {
	//	return;
	//}

	// ragePro systems can't fade blends, so don't obscure the screen
	if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO ) {
		return;
	}

	maxTime = DAMAGE_TIME;
	t = cg.time - cg.damageTime;
	if ( t <= 0 || t >= maxTime ) {
		return;
	}


	memset( &ent, 0, sizeof( ent ) );
	ent.reType = RT_SPRITE;
	ent.renderfx = RF_FIRST_PERSON;

	VectorMA( cg.refdef.vieworg, 8, cg.refdef.viewaxis[0], ent.origin );
	VectorMA( ent.origin, cg.damageX * -8, cg.refdef.viewaxis[1], ent.origin );
	VectorMA( ent.origin, cg.damageY * 8, cg.refdef.viewaxis[2], ent.origin );

	ent.radius = cg.damageValue * 3;
	ent.customShader = cgs.media.viewBloodShader;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 200 * ( 1.0 - ((float)t / maxTime) );
	trap_R_AddRefEntityToScene( &ent );
}


// see cg_local.h
qboolean cg_killcamRenderingFirstPerson = qfalse;

#ifndef KILLCAM_NO_MISSILE_CHASE
// missile-chase camera "hold": where the camera was when the missile
// exploded; it stays there watching the victim for the rest of the replay
static vec3_t	cg_killcamMissileHoldOrg;
static qboolean	cg_killcamMissileHoldValid = qfalse;

// where the killcam camera was on the previous killcam frame, whichever
// camera it was; used to derive the missile-chase offsets so the cut to
// the missile camera doesn't make the camera jump
static vec3_t	cg_killcamViewOrg;
static qboolean	cg_killcamViewOrgValid = qfalse;

// missile-chase offsets derived at the moment the chase begins
// (used unless overridden by the cg_killcamMissile* cvars)
static float	cg_killcamMissileAutoRange;
static float	cg_killcamMissileAutoHeight;
static float	cg_killcamMissileAutoSide;
static qboolean	cg_killcamMissileParamsValid = qfalse;

// the chased missile's last flight direction, kept so the camera
// doesn't jump when the missile comes to rest (e.g. a grenade waiting
// out its fuse becomes TR_STATIONARY, losing its velocity)
static vec3_t	cg_killcamMissileLastDir;
static qboolean	cg_killcamMissileLastDirValid = qfalse;

// fallbacks for the derived offsets, when there is no previous camera
// position to derive from (or it is degenerate)
#define KILLCAM_MISSILE_DEFAULT_RANGE	48
#define KILLCAM_MISSILE_DEFAULT_HEIGHT	12
#define KILLCAM_MISSILE_DEFAULT_SIDE	-15
#endif // KILLCAM_NO_MISSILE_CHASE

/*
===============
CG_KillcamViewReset

Called by CG_KillcamStart so per-replay camera state can't leak from a
previous replay
===============
*/
void CG_KillcamViewReset( void ) {
#ifndef KILLCAM_NO_MISSILE_CHASE
	cg_killcamMissileHoldValid = qfalse;
	cg_killcamViewOrgValid = qfalse;
	cg_killcamMissileParamsValid = qfalse;
	cg_killcamMissileLastDirValid = qfalse;
#endif // KILLCAM_NO_MISSILE_CHASE
}

/*
===============
CG_KillcamTargetPoint

The point on the victim that killcam cameras aim at: head height,
raised by cg_killcamHeight so that a camera raised by the same amount
looks horizontally when level with the victim
===============
*/
static void CG_KillcamTargetPoint( vec3_t target ) {
	VectorCopy( cg.predictedPlayerState.origin, target );
	target[2] += DEFAULT_VIEWHEIGHT + cg_killcamHeight.value;
}

#ifndef KILLCAM_NO_MISSILE_CHASE
/*
===============
CG_KillcamCalcMissileView

Death replay camera chasing the missile that scored the kill, looking
along its flight direction. After the explosion the camera holds its
last chase position, watching the victim (and their gibs). Returns
qfalse before the missile appears, or when it's out of the victim's
recorded PVS -- the caller falls back to the killer cameras.
===============
*/
static qboolean CG_KillcamCalcMissileView( void ) {
	static const vec3_t	camMins = { -6, -6, -6 };
	static const vec3_t	camMaxs = { 6, 6, 6 };
	centity_t	*missile;
	trace_t		trace;
	vec3_t		dir, camOrg, target;
	int			missileNum;

	missileNum = CG_KillcamMissileNum();
	if ( missileNum < 0 ) {
		return qfalse;
	}

	if ( cg.time < CG_KillcamMissileStartTime() ) {
		// back then, this entity number belonged to a different missile
		// (the server reuses entity numbers): don't chase that one
		return qfalse;
	}

	if ( cg.time >= CG_KillcamMissileExplodeTime() ) {
		// after the explosion: hold the last chase position, watching
		// the victim (and their gibs)
		if ( !cg_killcamMissileHoldValid ) {
			return qfalse;
		}
		VectorCopy( cg_killcamMissileHoldOrg, cg.refdef.vieworg );
		CG_KillcamTargetPoint( target );
		VectorSubtract( target, cg.refdef.vieworg, dir );
		if ( VectorNormalize( dir ) < 1 ) {
			return qfalse;
		}
		vectoangles( dir, cg.refdefViewAngles );
		return qtrue;
	}

	missile = &cg_entities[missileNum];
	if ( !missile->currentValid || missile->currentState.eType != ET_MISSILE ) {
		// not fired yet, or out of the victim's recorded PVS
		return qfalse;
	}

	// see the comment in CG_KillcamCalcKillerView
	CG_SetFrameInterpolation();
	CG_CalcEntityLerpPositions( missile );

	// the chase position is behind the missile along its flight direction
	BG_EvaluateTrajectoryDelta( &missile->currentState.pos, cg.time, dir );
	if ( VectorNormalize( dir ) < 1 ) {
		// near-stationary (e.g. a grenade at rest waiting out its fuse)
		if ( cg_killcamMissileLastDirValid ) {
			// keep the direction it was flying in, so the camera
			// doesn't jump the moment the missile stops
			VectorCopy( cg_killcamMissileLastDir, dir );
		} else {
			// never seen flying: place the camera on the far side
			// from the victim
			VectorSubtract( cg.predictedPlayerState.origin, missile->lerpOrigin, dir );
			if ( VectorNormalize( dir ) < 1 ) {
				return qfalse;
			}
		}
	} else {
		VectorCopy( dir, cg_killcamMissileLastDir );
		cg_killcamMissileLastDirValid = qtrue;
	}
	vectoangles( dir, cg.refdefViewAngles );

	// chase from behind, slightly above and to the side, without going
	// into walls. The chase position trails the *horizontal* flight
	// direction only: a floor bounce flips the vertical velocity, and
	// trailing the full direction would put the camera under the floor
	// (the wall trace then pins it onto the grenade itself)
	{
		float		range, height, side;
		vec3_t		dirH;
		vec3_t		right;
		qboolean	haveRight;

		dirH[0] = dir[0];
		dirH[1] = dir[1];
		dirH[2] = 0;
		if ( VectorNormalize( dirH ) < 0.1f ) {
			// near-vertical flight: no meaningful horizontal direction
			VectorCopy( dir, dirH );
		}

		// horizontal perpendicular of the flight direction, same
		// convention as cg_killcamSide (positive = to the right)
		right[0] = dirH[1];
		right[1] = -dirH[0];
		right[2] = 0;
		haveRight = VectorNormalize( right ) > 0.1f;

		if ( !cg_killcamMissileParamsValid ) {
			// derive the chase offsets from where the camera is right
			// now (the previous frame's killcam camera, whichever it
			// was), so the camera doesn't jump at the cut
			cg_killcamMissileAutoRange = KILLCAM_MISSILE_DEFAULT_RANGE;
			cg_killcamMissileAutoHeight = KILLCAM_MISSILE_DEFAULT_HEIGHT;
			cg_killcamMissileAutoSide = KILLCAM_MISSILE_DEFAULT_SIDE;
			if ( cg_killcamViewOrgValid && haveRight ) {
				vec3_t	delta;

				// decompose (previous camera - missile) in the frame
				// the chase position is composed in below:
				// delta = -range*dirH + height*up + side*right
				VectorSubtract( cg_killcamViewOrg, missile->lerpOrigin, delta );
				cg_killcamMissileAutoSide = DotProduct( delta, right );
				cg_killcamMissileAutoRange =
					-( delta[0] * dirH[0] + delta[1] * dirH[1] );
				cg_killcamMissileAutoHeight = delta[2];

				// keep the derived offsets sane: the previous camera
				// can be anywhere (e.g. the victim's own view far from
				// the launch point)
				if ( cg_killcamMissileAutoRange < 8 ) cg_killcamMissileAutoRange = 8;
				if ( cg_killcamMissileAutoRange > 120 ) cg_killcamMissileAutoRange = 120;
				if ( cg_killcamMissileAutoHeight < -30 ) cg_killcamMissileAutoHeight = -30;
				if ( cg_killcamMissileAutoHeight > 60 ) cg_killcamMissileAutoHeight = 60;
				if ( cg_killcamMissileAutoSide < -60 ) cg_killcamMissileAutoSide = -60;
				if ( cg_killcamMissileAutoSide > 60 ) cg_killcamMissileAutoSide = 60;
			}
			cg_killcamMissileParamsValid = qtrue;
		}

		// the cvars, when set, override the derived offsets
		range = cg_killcamMissileRange.string[0] != '\0'
			? cg_killcamMissileRange.value : cg_killcamMissileAutoRange;
		height = cg_killcamMissileHeight.string[0] != '\0'
			? cg_killcamMissileHeight.value : cg_killcamMissileAutoHeight;
		side = cg_killcamMissileSide.string[0] != '\0'
			? cg_killcamMissileSide.value : cg_killcamMissileAutoSide;

		VectorMA( missile->lerpOrigin, -range, dirH, camOrg );
		camOrg[2] += height;
		if ( side != 0 && haveRight ) {
			VectorMA( camOrg, side, right, camOrg );
		}
	}
	CG_Trace( &trace, missile->lerpOrigin, camMins, camMaxs, camOrg, missileNum, MASK_SOLID );
	VectorCopy( trace.endpos, cg.refdef.vieworg );

	// where to look: along the flight direction (already set above), or
	// at the target -- the default for grenades, whose lobbed arcs
	// rarely point at the victim
	if ( cg_killcamMissileLookAtTarget.integer == 1 ||
		( cg_killcamMissileLookAtTarget.integer == 2 &&
			missile->currentState.weapon == WP_GRENADE_LAUNCHER ) )
	{
		CG_KillcamTargetPoint( target );
		VectorSubtract( target, cg.refdef.vieworg, dir );
		if ( VectorNormalize( dir ) >= 1 ) {
			vectoangles( dir, cg.refdefViewAngles );
		}
	}

	VectorCopy( cg.refdef.vieworg, cg_killcamMissileHoldOrg );
	cg_killcamMissileHoldValid = qtrue;

	// the victim's bob state doesn't apply here
	cg.bobcycle = 0;
	cg.bobfracsin = 0;
	cg.xyspeed = 0;

	return qtrue;
}
#endif // KILLCAM_NO_MISSILE_CHASE

/*
===============
CG_KillcamCalcKillerFirstPersonView

Death replay camera from the killer's eyes. Returns qfalse (falling
back to the third-person killer camera) if the killer isn't in the
replayed snapshot or is dead.
===============
*/
static qboolean CG_KillcamCalcKillerFirstPersonView( void ) {
	centity_t	*killer;
	int			killerNum;
	int			legsAnim;

	killerNum = CG_KillcamKillerNum();
	if ( killerNum < 0 || killerNum >= MAX_CLIENTS ||
		killerNum == cg.snap->ps.clientNum )
	{
		return qfalse;
	}

	killer = &cg_entities[killerNum];
	if ( !killer->currentValid ) {
		return qfalse;
	}
	if ( killer->currentState.eFlags & EF_DEAD ) {
		// first person from a corpse (mutual kill) looks broken
		return qfalse;
	}

	// see the comment in CG_KillcamCalcKillerView
	CG_SetFrameInterpolation();
	CG_CalcEntityLerpPositions( killer );

	VectorCopy( killer->lerpOrigin, cg.refdef.vieworg );
	legsAnim = killer->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if ( legsAnim == LEGS_WALKCR || legsAnim == LEGS_IDLECR ) {
		cg.refdef.vieworg[2] += CROUCH_VIEWHEIGHT;
	} else {
		cg.refdef.vieworg[2] += DEFAULT_VIEWHEIGHT;
	}
	VectorCopy( killer->lerpAngles, cg.refdefViewAngles );

	// the victim's bob state doesn't apply to the killer's view weapon
	cg.bobcycle = 0;
	cg.bobfracsin = 0;
	cg.xyspeed = 0;

	return qtrue;
}

/*
===============
CG_KillcamCalcKillerView

Death replay camera: place the camera at (slightly behind) the killer,
aiming at the victim -- the recorded local player. Returns qfalse if the
killer isn't in the replayed snapshot (out of the victim's PVS), in
which case the caller keeps the normal view of the victim.
===============
*/
static qboolean CG_KillcamCalcKillerView( void ) {
	static const vec3_t	camMins = { -6, -6, -6 };
	static const vec3_t	camMaxs = { 6, 6, 6 };
	centity_t	*killer;
	trace_t		trace;
	vec3_t		eye, target, forward, camOrg;
	int			killerNum;

	killerNum = CG_KillcamKillerNum();
	if ( killerNum < 0 || killerNum >= MAX_CLIENTS ||
		killerNum == cg.snap->ps.clientNum )
	{
		return qfalse;
	}

	killer = &cg_entities[killerNum];
	if ( !killer->currentValid ) {
		return qfalse;
	}

	// cg.frameInterpolation is normally set later in the frame, by
	// CG_AddPacketEntities. Without this the killer's lerpOrigin here
	// is computed with the previous frame's interpolation fraction and
	// disagrees with where the killer model is actually drawn, which
	// makes the camera shake.
	CG_SetFrameInterpolation();
	CG_CalcEntityLerpPositions( killer );
	VectorCopy( killer->lerpOrigin, eye );
	eye[2] += DEFAULT_VIEWHEIGHT;

	CG_KillcamTargetPoint( target );

	// raise the camera above the killer's head, tracing so that a low
	// ceiling doesn't put it in solid
	VectorCopy( eye, camOrg );
	camOrg[2] += cg_killcamHeight.value;
	CG_Trace( &trace, eye, camMins, camMaxs, camOrg, killerNum, MASK_SOLID );
	VectorCopy( trace.endpos, eye );

	// and shift it sideways, so that neither the killer's model nor the
	// award icons above their head cover the victim
	if ( cg_killcamSide.value != 0 ) {
		vec3_t	right;

		VectorSubtract( target, eye, forward );
		forward[2] = 0;
		if ( VectorNormalize( forward ) >= 1 ) {
			right[0] = forward[1];
			right[1] = -forward[0];
			right[2] = 0;
			VectorMA( eye, cg_killcamSide.value, right, camOrg );
			CG_Trace( &trace, eye, camMins, camMaxs, camOrg, killerNum, MASK_SOLID );
			VectorCopy( trace.endpos, eye );
		}
	}

	VectorSubtract( target, eye, forward );
	if ( VectorNormalize( forward ) < 1 ) {
		// killer is right on top of the victim
		return qfalse;
	}
	vectoangles( forward, cg.refdefViewAngles );

	// back away from the killer's head so their model is visible,
	// without going into a wall
	VectorMA( eye, -cg_killcamRange.value, forward, camOrg );
	CG_Trace( &trace, eye, camMins, camMaxs, camOrg, killerNum, MASK_SOLID );
	VectorCopy( trace.endpos, cg.refdef.vieworg );

	return qtrue;
}

/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues( void ) {
	playerState_t	*ps;
	qboolean		killcamCameraPlaced;

	memset( &cg.refdef, 0, sizeof( cg.refdef ) );

	// strings for in game rendering
	// Q_strncpyz( cg.refdef.text[0], "Park Ranger", sizeof(cg.refdef.text[0]) );
	// Q_strncpyz( cg.refdef.text[1], "19", sizeof(cg.refdef.text[1]) );

	// calculate size of 3D view
	CG_CalcVrect();

	ps = &cg.predictedPlayerState;
/*
	if (cg.cameraMode) {
		vec3_t origin, angles;
		if (trap_getCameraInfo(cg.time, &origin, &angles)) {
			VectorCopy(origin, cg.refdef.vieworg);
			angles[ROLL] = 0;
			VectorCopy(angles, cg.refdefViewAngles);
			AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
			return CG_CalcFov();
		} else {
			cg.cameraMode = qfalse;
		}
	}
*/
	// intermission view
	if ( ps->pm_type == PM_INTERMISSION ) {
		VectorCopy( ps->origin, cg.refdef.vieworg );
		VectorCopy( ps->viewangles, cg.refdefViewAngles );
		AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
		return CG_CalcFov();
	}

	cg.bobcycle = ( ps->bobCycle & 128 ) >> 7;
	cg.bobfracsin = fabs( sin( ( ps->bobCycle & 127 ) / 127.0 * M_PI ) );
	cg.xyspeed = sqrt( ps->velocity[0] * ps->velocity[0] +
		ps->velocity[1] * ps->velocity[1] );


	VectorCopy( ps->origin, cg.refdef.vieworg );
	VectorCopy( ps->viewangles, cg.refdefViewAngles );

	if (cg_cameraOrbit.integer) {
		if (cg.time > cg.nextOrbitTime) {
			cg.nextOrbitTime = cg.time + cg_cameraOrbitDelay.integer;
			cg_thirdPersonAngle.value += cg_cameraOrbit.value;
		}
	}
	// add error decay
	if ( cg_errorDecay.value > 0 ) {
		int		t;
		float	f;

		t = cg.time - cg.predictedErrorTime;
		f = ( cg_errorDecay.value - t ) / cg_errorDecay.value;
		if ( f > 0 && f < 1 ) {
			VectorMA( cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg );
		} else {
			cg.predictedErrorTime = 0;
		}
	}

	cg_killcamRenderingFirstPerson = qfalse;
	killcamCameraPlaced = qfalse;
	if ( cg_contextNum == CG_CONTEXT_KILLCAM && CG_KillcamMode() == KILLCAM_KILLER ) {
#ifndef KILLCAM_NO_MISSILE_CHASE
		if ( CG_KillcamCalcMissileView() ) {
			// camera is chasing the killing missile
			killcamCameraPlaced = qtrue;
		} else
#endif // KILLCAM_NO_MISSILE_CHASE
		if ( cg_killcamFirstPerson.integer &&
			CG_KillcamCalcKillerFirstPersonView() )
		{
			// camera was placed at the killer's eyes
			cg_killcamRenderingFirstPerson = qtrue;
			killcamCameraPlaced = qtrue;
		} else if ( CG_KillcamCalcKillerView() ) {
			// camera was placed at the killer
			killcamCameraPlaced = qtrue;
		}
	}

	if ( killcamCameraPlaced ) {
		// the victim must be drawn: the camera is looking at them
		cg.renderingThirdPerson = qtrue;
	} else if ( cg.renderingThirdPerson ) {
		// back away from character
		CG_OffsetThirdPersonView();
	} else {
		// offset for local bobbing and kicks
		CG_OffsetFirstPersonView();
	}

#ifndef KILLCAM_NO_MISSILE_CHASE
	// remember where the killcam camera ended up, whichever camera it
	// was, so the missile chase can pick up from here without a jump
	if ( cg_contextNum == CG_CONTEXT_KILLCAM && CG_KillcamMode() == KILLCAM_KILLER ) {
		VectorCopy( cg.refdef.vieworg, cg_killcamViewOrg );
		cg_killcamViewOrgValid = qtrue;
	}
#endif // KILLCAM_NO_MISSILE_CHASE

	// position eye relative to origin
	AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );

	if ( cg.hyperspace ) {
		cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
	}

	// field of view
	return CG_CalcFov();
}


/*
=====================
CG_PowerupTimerSounds
=====================
*/
static void CG_PowerupTimerSounds( void ) {
	int		i;
	int		t;

	// powerup timers going away
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		t = cg.snap->ps.powerups[i];
		if ( t <= cg.time ) {
			continue;
		}
		if ( t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME ) {
			continue;
		}
		if ( ( t - cg.time ) / POWERUP_BLINK_TIME != ( t - cg.oldTime ) / POWERUP_BLINK_TIME ) {
			trap_S_StartSound( NULL, cg.snap->ps.clientNum, CHAN_ITEM, cgs.media.wearOffSound );
		}
	}
}

/*
=====================
CG_AddBufferedSound
=====================
*/
void CG_AddBufferedSound( sfxHandle_t sfx ) {
	if ( !sfx )
		return;

	// clear all buffered sounds
	if ( sfx == -1 ) {
		cg.soundBufferIn = 0;
		cg.soundBufferOut = 0;
		memset( cg.soundBuffer, 0, sizeof( cg.soundBuffer ) );
		return;
	}

	cg.soundBuffer[cg.soundBufferIn] = sfx;
	cg.soundBufferIn = (cg.soundBufferIn + 1) % MAX_SOUNDBUFFER;
	if (cg.soundBufferIn == cg.soundBufferOut) {
		//cg.soundBufferOut++;
		cg.soundBufferOut = (cg.soundBufferOut + 1) % MAX_SOUNDBUFFER;
	}
}

/*
=====================
CG_PlayBufferedSounds
=====================
*/
static void CG_PlayBufferedSounds( void ) {
	if ( cg.soundTime < cg.time ) {
		if (cg.soundBufferOut != cg.soundBufferIn && cg.soundBuffer[cg.soundBufferOut]) {
			cg.soundPlaying = cg.soundBuffer[cg.soundBufferOut];
			trap_S_StartLocalSound( cg.soundPlaying, CHAN_ANNOUNCER );
			cg.soundBuffer[cg.soundBufferOut] = 0;
			cg.soundBufferOut = (cg.soundBufferOut + 1) % MAX_SOUNDBUFFER;
			cg.soundTime = cg.time + 750;
		} else {
			cg.soundPlaying = 0;
		}
	}
}

//=========================================================================


/*
=================
CG_FirstFrame

Called once on first rendered frame
=================
*/
static void CG_FirstFrame( void )
{
	CG_SetConfigValues();

	cgs.voteTime = atoi( CG_ConfigString( CS_VOTE_TIME ) );
	cgs.voteYes = atoi( CG_ConfigString( CS_VOTE_YES ) );
	cgs.voteNo = atoi( CG_ConfigString( CS_VOTE_NO ) );
	Q_strncpyz( cgs.voteString, CG_ConfigString( CS_VOTE_STRING ), sizeof( cgs.voteString ) );

	if ( cgs.voteTime )
		cgs.voteModified = qtrue;
	else
		cgs.voteModified = qfalse;
}


// qtrue while the live context is processed hidden behind the killcam
// replay: no rendering, no engine-global side effects (its sounds are
// muted separately via cg_soundMuted)
static qboolean cg_passHidden = qfalse;

/*
=================
CG_DrawActiveFrameCtx

Processes the current context up to the given time and, unless this is
a hidden pass, generates and draws its scene and status information.
=================
*/
static void CG_DrawActiveFrameCtx( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {
	int		inwater;

	cg.time = serverTime;
	cg.demoPlayback = demoPlayback;

	if ( !cg_passHidden ) {
		// any looped sounds will be respecified as entities
		// are added to the render list
		trap_S_ClearLoopingSounds(qfalse);

		// clear all the render lists
		trap_R_ClearScene();
	}

	// set up cg.snap and possibly cg.nextSnap
	CG_ProcessSnapshots();

	// if we haven't received any snapshots yet, all
	// we can draw is the information screen
	if ( !cg.snap || ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
		if ( !cg_passHidden ) {
			CG_DrawInformation();
		}
		return;
	}

	if ( cg_contextNum == CG_CONTEXT_LIVE ) {
		// let the client system know what our weapon and zoom settings are
		trap_SetUserCmdValue( cg.weaponSelect, cg.zoomSensitivity );

		if ( cg.clientFrame == 0 )
			CG_FirstFrame();
	}

	// update cg.predictedPlayerState
	CG_PredictPlayerState();

	// decide on third person view
	cg.renderingThirdPerson = cg_thirdPerson.integer || (cg.snap->ps.stats[STAT_HEALTH] <= 0);

	// note: when the killcam manages to place a camera looking at the
	// victim, CG_CalcViewValues forces renderingThirdPerson so the
	// victim's body is drawn; when it can't (killer not in the recorded
	// data), this default stands and the victim gets their own normal
	// view -- first person while still alive in the replay

	if ( cg_contextNum == CG_CONTEXT_LIVE ) {
		CG_TrackClientTeamChange();
	}

	// follow killer
	if ( cg.followTime && cg.followTime < cg.time ) {
		cg.followTime = 0;
		if ( !cg.demoPlayback ) {
			trap_SendConsoleCommand( va( "follow %i\n", cg.followClient ) );
		}
	}

	// build cg.refdef
	inwater = CG_CalcViewValues();

	if ( !cg_passHidden ) {
		// first person blend blobs, done after AnglesToAxis
		if ( !cg.renderingThirdPerson ) {
			CG_DamageBlendBlob();
		}

		// build the render lists
		if ( !cg.hyperspace ) {
			CG_AddPacketEntities();	// alter calcViewValues, so predicted player state is correct
			CG_AddMarks();
			CG_AddParticles ();
		}
	}
	// Local entities (gibs!) carry their own physics state -- bouncing
	// rewrites their trajectory -- so they must keep being simulated
	// even while this context is processed hidden behind the killcam.
	// Otherwise they freeze and, when the view switches back, get hit
	// with all the accumulated gravity at once and plummet straight
	// down. In a hidden pass the refEntities added here are discarded
	// by the killcam pass's trap_R_ClearScene, and the bounce sounds
	// are muted via cg_soundMuted.
	if ( !cg.hyperspace ) {
		CG_AddLocalEntities();
	}
	if ( !cg_passHidden ) {
		if ( cg_killcamRenderingFirstPerson ) {
			// the killer's weapon; the normal view weapon is skipped
			// anyway because the killcam forces renderingThirdPerson
			CG_KillcamAddViewWeapon();
		} else {
			CG_AddViewWeapon( &cg.predictedPlayerState );
		}
	}

	// add buffered sounds
	CG_PlayBufferedSounds();

#ifdef MISSIONPACK
	// play buffered voice chats
	CG_PlayBufferedVoiceChats();
#endif

	// finish up the rest of the refdef
	if ( !cg_passHidden && cg.testModelEntity.hModel ) {
		CG_AddTestModel();
	}
	cg.refdef.time = cg.time;
	memcpy( cg.refdef.areamask, cg.snap->areamask, sizeof( cg.refdef.areamask ) );

	// warning sounds when powerup is wearing off
	CG_PowerupTimerSounds();

	if ( !cg_passHidden ) {
		// update audio positions
		trap_S_Respatialize( cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater );
	}

	// make sure the lagometerSample and frame timing isn't done twice when in stereo
	if ( stereoView != STEREO_RIGHT ) {
		cg.frametime = cg.time - cg.oldTime;
		if ( cg.frametime < 0 ) {
			cg.frametime = 0;
		}
		cg.oldTime = cg.time;
		if ( cg_contextNum == CG_CONTEXT_LIVE ) {
			CG_AddLagometerFrameInfo();
		}
	}
	if ( cg_contextNum == CG_CONTEXT_LIVE ) {
		if (cg_timescale.value != cg_timescaleFadeEnd.value) {
			if (cg_timescale.value < cg_timescaleFadeEnd.value) {
				cg_timescale.value += cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
				if (cg_timescale.value > cg_timescaleFadeEnd.value)
					cg_timescale.value = cg_timescaleFadeEnd.value;
			}
			else {
				cg_timescale.value -= cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
				if (cg_timescale.value < cg_timescaleFadeEnd.value)
					cg_timescale.value = cg_timescaleFadeEnd.value;
			}
			if (cg_timescaleFadeSpeed.value) {
				trap_Cvar_Set("timescale", va("%f", cg_timescale.value));
			}
		}
	}

	if ( !cg_passHidden ) {
		// actually issue the rendering calls
		CG_DrawActive( stereoView );
	}

	// this counter will be bumped for every valid scene we generate
	cg.clientFrame++;

	if ( cg_stats.integer ) {
		CG_Printf( "cg.clientFrame:%i\n", cg.clientFrame );
	}
}

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.

When the killcam is active, the live context is still processed every
frame (hidden and muted, so its state stays current), and the killcam
context is processed and rendered at a delayed time from recorded
snapshots.
=================
*/
void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {
	int			killcamDelay;
	qboolean	killcamView;

	CG_SetContext( CG_CONTEXT_LIVE );

	// update cvars
	CG_UpdateCvars();

	// if we are only updating the screen as a loading
	// pacifier, don't even try to read snapshots
	if ( cg.infoScreenText[0] != 0 ) {
		CG_DrawInformation();
		return;
	}

	// killcam: death replay / test mode
	killcamDelay = CG_KillcamUpdate( serverTime );
	killcamView = killcamDelay > 0;

	cg_passHidden = killcamView;
	cg_soundMuted = killcamView;
	CG_DrawActiveFrameCtx( serverTime, stereoView, demoPlayback );
	cg_passHidden = qfalse;
	cg_soundMuted = qfalse;

	if ( killcamView ) {
		CG_SetContext( CG_CONTEXT_KILLCAM );
		// the replayed stream is conceptually a demo
		CG_DrawActiveFrameCtx( serverTime - killcamDelay, stereoView, qtrue );
		CG_SetContext( CG_CONTEXT_LIVE );
	}
}
