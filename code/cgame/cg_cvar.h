#ifdef EXTERN_CG_CVAR
	#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags, description ) extern vmCvar_t vmCvar;
#endif

#ifdef DECLARE_CG_CVAR
	#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags, description ) vmCvar_t vmCvar;
#endif

#ifdef CG_CVAR_LIST
	#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags, description ) { & vmCvar, cvarName, defaultString, cvarFlags, description },
#endif

CG_CVAR( cg_ignore, "cg_ignore", "0", 0, NULL ) // used for debugging
CG_CVAR( cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_fov, "cg_fov", "90", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_gibs, "cg_gibs", "1.0", CVAR_ARCHIVE,
	"Make a bloody explosion when a player takes a lot of damage.\n"
	"The bigger the value the more pieces." )
CG_CVAR( cg_oldGibs, "cg_oldGibs", "0", CVAR_ARCHIVE,
	"Vanilla gibs, from 1999. "
	"When set to 1, takes priority "
	"over (almost) all the other `cg_gibs*` vars." )
CG_CVAR( cg_gibsInheritPlayerVelocity, "cg_gibsInheritPlayerVelocity", "1.0", CVAR_ARCHIVE,
	"If the player is moving fast, their gibs will also move fast. "
	"And the player will move fast when they receive damage (knockback). "
	"The most important var of the Better Gibs mod.\n"
	"\n"
	"It's possible to set a value other than 1 or 0, "
	"but if you simply want the gibs to have more speed, "
	"you're probably after `cg_gibsKnockback` or other `cg_gibs` vars." )
CG_CVAR( cg_gibsKnockback, "cg_gibsKnockback", "1.0", 0,
	"Scale the knockback when gibbing a player, "
	"as if they received more (or less) damage.\n"
	"On vanilla servers this only has effect on random and vertical velocity "
	"but not on directional velocity.\n"
	"Remember that this also affects own gibs, so having a value too high "
	"will make gibs fly faster than the camera." )
CG_CVAR( cg_gibsExtraKnockback, "cg_gibsExtraKnockback", "100", 0,
	"Like `cg_gibsKnockback`, but adds a *fixed* amount of knockback. "
	"100 of knockback corresponds to 20 of damage.\n"
	"This basically sets a \"baseline\" amount of knockback, "
	"making all shots feel powerful, "
	"while not scaling the already high speed the gibs would gain "
	"from Quad shots." )
CG_CVAR( cg_gibsPiecesFromKnockback, "cg_gibsPiecesFromKnockback", "1.1", CVAR_ARCHIVE,
	"For every 100 of knockback speed above 500 (100 damage), "
	"add this many pieces of gibs, times the value of `cg_gibs`.\n"
	"This makes for more pieces when someone gets gibbed by Quad.\n"
	"\n"
	"With higher damage gibs fly further apart, so one starts to see gibs "
	"as individual pieces instead of a \"cloud\", which IMO is not good. "
	"Increasing the amount of gibs with damage fixes this. "
	"And overall it makes it more rewarding to deal more damage.\n"
	"Q: \"What sense does it make to have more gibs? "
	"Did the player got bigger from taking more damage??\":\n"
	"A: No, they simlpy got split into more pieces." )
CG_CVAR( cg_gibsLinearVelocityFromKnockback, "cg_gibsLinearVelocityFromKnockback", "1", 0,
	"Scale how fast the gibs move away from damage (knockback).\n"
	"This sounds similar to `cg_gibsInheritPlayerVelocity`, "
	"but this var doesn't scale the velocity that the player already had "
	"when they received the damage." )
CG_CVAR( cg_gibsRandomVelocityFromKnockback, "cg_gibsRandomVelocityFromKnockback", "0.3", CVAR_ARCHIVE,
	"Scale how much random velocity (spread) the gibs gain "
	"from damage (knockback)." )
CG_CVAR( cg_gibsVerticalVelocityFromKnockback, "cg_gibsVerticalVelocityFromKnockback", "0.2", CVAR_ARCHIVE,
	"Scale how much vertical velocity the gibs gain from damage (knockback)." )
CG_CVAR( cg_gibsExtraRandomVelocity, "cg_gibsExtraRandomVelocity", "100", CVAR_ARCHIVE,
	"Like `cg_gibsRandomVelocityFromKnockback`, "
	"but add a *fixed* amount of random velocity to each piece.\n"
	"Ensures that even if the player was gibbed with the machine gun, "
	"the gibs would still fly apart somewhat." )
CG_CVAR( cg_gibsExtraVerticalVelocity, "cg_gibsExtraVerticalVelocity", "50", CVAR_ARCHIVE,
	"Like `cg_gibsVerticalVelocityFromKnockback`, "
	"but adds a *fixed* amount of vertical speed to each piece" )
CG_CVAR( cg_gibsPlayerSpeedFromKnockback, "cg_gibsPlayerSpeedFromKnockback", "-0.25", 0,
	"How much to slow down the player "
	"depending on how much damage (knockback) they took.\n"
	"This makes the original player velocity have less effect "
	"on the final gib velocity, thus giving more emphasis to the damage direction.\n"
	"\n"
	"Unlike reducing `cg_gibsInheritPlayerVelocity`, "
	"this works better if the player is already moving at high speed "
	"(e.g. on a jump pad), or falling. "
	"This will not reduce player velocity if the knockback was low "
	"or if damage had no direction (e.g. falling).\n"
	"\n"
	"Has no effect on vanilla servers." )
CG_CVAR( cg_gibsPlayerSpeedFromKnockbackMaxFraction, "cg_gibsPlayerSpeedFromKnockbackMaxFraction", "-0.5", 0,
	"See `cg_gibsPlayerSpeedFromKnockback`. "
	"If knockback is big, this helps keep at least some fraction "
	"of player speed. "
	"For example, if this is -0.75, we'll always keep at least 25% "
	"of original player velocity." )
CG_CVAR( cg_gibsDuration, "cg_gibsDuration", "5000", 0,
	"How long (in milliseconds) gibs stay around "
	"before sinking into the ground." )
CG_CVAR( cg_gibsOriginalOrigin, "cg_gibsOriginalOrigin", "0.0", 0,
	"Whether to use the exact position where the player got gibbed "
	"as the initial gibs position, instead of the position "
	"of the gibbed player on the screen, i.e. the position of the player "
	"on the next server frame (lerpOrigin).\n"
	"\n"
	"Can be a fractional value between 0 and 1, "
	"but you probably want either 0 or 1.\n"
	"To get vanilla behavior, set to 0.\n"
	"\n"
	"Has no effect on vanilla servers (`g_gibsNewEvGibPlayerProtocol 0`).\n"
	"Basically has no effect when `cg_gibsNoLerpDelay 1`.\n"
	"\n"
	"Using the original position might make it seem\n"
	"like the player \"telepoted\" one snapshot back (50ms at `snaps 20`)\n"
	"if they're already moving at high speed." )
CG_CVAR( cg_gibsNoLerpDelay, "cg_gibsNoLerpDelay", "1", 0,
	"Don't wait for next snapshot to arrive (normally 50ms) "
	"before gibbing players.\n"
	"Otherwise after taking a shot the player's body will stay intact "
	"for the said 50ms, then get gibbed.\n"
	"Consequently, this affects the position from which the gibs are launched, "
	"due to the gibbed player (or, rather, their camera) "
	"moving during those 50ms. See `cg_gibsOriginalOrigin`.\n"
	"\n"
	"Note that when looking at another player (or missile) gibbing someone "
	"it will look like the player gets gibbed "
	"50ms before the actual shot is fired.\n"
	"That is, the gib event will happen "
	"ahead of the displayed state of the game.\n"
	"\n"
	"Set this to 3 (2nd bit) to also do this on vanilla servers.\n"
	"Set this to 5 (3rd bit) to keep the body visible "
	"until the next snapshot. That is, both the body and the gibs "
	"will be visible together for ~50ms.\n"
	"To get vanilla behavior, set to 0." )
CG_CVAR( cg_gibsStopPlayerSounds, "cg_gibsStopPlayerSounds", "3", 0,
	"Stop player's sounds (pain, taunts) when they get gibbed.\n"
	"Doesn't work for self as of writing. "
	"Set 1st bit to apply this to other players. "
	"Set 2nd bit to still try to apply this to self." )
CG_CVAR( cg_gibsMaxSimultaneousBounceSounds, "cg_gibsMaxSimultaneousBounceSounds", "3", 0,
	"Limit the amount of \"splat\" sounds per a few milliseconds" )
CG_CVAR( cg_gibsBounceFactor, "cg_gibsBounceFactor", "0.4", CVAR_ARCHIVE,
	"Fraction of speed that gibs preserve when bouncing off a surface" )
CG_CVAR( cg_gibsBounceFactorRandomness, "cg_gibsBounceFactorRandomness", "0.5", CVAR_ARCHIVE,
	"Fraction of `cg_gibsBounceFactor` that is random. "
	"This makes gibs look less uniform, and spread a little more." )
CG_CVAR( cg_gibsRotationFactor, "cg_gibsRotationFactor", "1.0", CVAR_ARCHIVE,
	"Scale how fast gib pieces will rotate, depending on damage (knockback)." )
// Not archived because experimental
CG_CVAR( cg_gibsBloodTrailPeriod, "cg_gibsBloodTrailPeriod", "150", 0,
	"How often in milliseconds gibs leave blood \"clouds\"" )
CG_CVAR( cg_gibsFireTrailPeriod, "cg_gibsFireTrailPeriod", "30", 0,
	"How often in milliseconds gibs leave fire \"clouds\".\n"
	"Remember to set `cg_gibsFireTrailMinKnockback` >= 0 for this to work" )
CG_CVAR( cg_gibsFireTrailDuration, "cg_gibsFireTrailDuration", "1024", 0,
	"How long in milliseconds gibs leave fire trails for.\n"
	"Remember to set `cg_gibsFireTrailMinKnockback` >= 0 for this to work" )
CG_CVAR( cg_gibsFireTrailMinKnockback, "cg_gibsFireTrailMinKnockback", "-1", 0,
	"How much knockback the player must receive "
	"for their gibs to leave fire trails.\n"
	"100 damage is 500 knockback.\n"
	"`-1` to disable." )
CG_CVAR( cg_gibsBetterCameraOnGib, "cg_gibsBetterCameraOnGib", "1", CVAR_USERINFO | CVAR_ARCHIVE,
	"When you get gibbed, makes your camera get knocked farther away "
	"thanks to it being smaller than your player's bounding box, "
	"and thus not having friction with the ground until it falls down.\n"
	"Mostly noticeable when getting gibbed with a shotgun or a railgun.\n"
	"Doesn't work on vanilla servers." )
CG_CVAR( cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE , NULL )
CG_CVAR( cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawSpeed, "cg_drawSpeed", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_drawWeaponSelect, "cg_drawWeaponSelect", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_crosshairHealth, "cg_crosshairHealth", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_bounceMarksMinImpactSpeed, "cg_bounceMarksMinImpactSpeed", "200", CVAR_ARCHIVE,
	"Note that ~290 corresponds to a free fall with no bounce from player height." )
CG_CVAR( cg_bounceSoundMinImpactSpeed, "cg_bounceSoundMinImpactSpeed", "450", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE , NULL )
CG_CVAR( cg_railTrailRadius, "cg_railTrailRadius", "0", CVAR_ARCHIVE , NULL )
CG_CVAR( cg_gun_frame, "cg_gun_frame", "", CVAR_ROM, NULL )
CG_CVAR( cg_gun_x, "cg_gunX", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_gun_y, "cg_gunY", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_gun_z, "cg_gunZ", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_centertime, "cg_centertime", "3", CVAR_CHEAT, NULL )
CG_CVAR( cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_bobup , "cg_bobup", "0.005", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT, NULL )
CG_CVAR( cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT, NULL )
CG_CVAR( cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT, NULL )
CG_CVAR( cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT, NULL )
CG_CVAR( cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT, NULL )
CG_CVAR( cg_debugGibs, "cg_debugGibs", "0", CVAR_CHEAT, NULL )
CG_CVAR( cg_errorDecay, "cg_errordecay", "100", 0, NULL )
CG_CVAR( cg_nopredict, "cg_nopredict", "0", 0, NULL )
CG_CVAR( cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT, NULL )
CG_CVAR( cg_showmiss, "cg_showmiss", "0", 0, NULL )
CG_CVAR( cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT, NULL )
CG_CVAR( cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT, NULL )
CG_CVAR( cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT, NULL )
CG_CVAR( cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT, NULL )
CG_CVAR( cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT, NULL )
CG_CVAR( cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT, NULL )
CG_CVAR( cg_thirdPerson, "cg_thirdPerson", "0", 0, NULL )
CG_CVAR( cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE , NULL )
CG_CVAR( cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE , NULL )
CG_CVAR( cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE , NULL )
CG_CVAR( cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE, NULL )
#ifdef MISSIONPACK
CG_CVAR( cg_deferPlayers, "cg_deferPlayers", "0", CVAR_ARCHIVE, NULL )
#else
CG_CVAR( cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE, NULL )
#endif
CG_CVAR( cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO, NULL )
CG_CVAR( cg_stats, "cg_stats", "0", 0, NULL )
CG_CVAR( cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE, NULL )
#ifdef MISSIONPACK
CG_CVAR( cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE, NULL )
#endif
// the following variables are created in other parts of the system,
// but we also reference them here
CG_CVAR( cg_buildScript, "com_buildScript", "0", 0, NULL )	// force loading of all possible data amd error on failures
CG_CVAR( cg_paused, "cl_paused", "0", CVAR_ROM, NULL )
CG_CVAR( cg_blood, "com_blood", "1", CVAR_ARCHIVE, NULL )
#ifdef MISSIONPACK
CG_CVAR( cg_redTeamName, "g_redteam", DEFAULT_REDTEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO, NULL )
CG_CVAR( cg_blueTeamName, "g_blueteam", DEFAULT_BLUETEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO, NULL )
CG_CVAR( cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_singlePlayer, "ui_singlePlayerActive", "0", CVAR_USERINFO, NULL )
CG_CVAR( cg_enableDust, "g_enableDust", "0", CVAR_SERVERINFO, NULL )
CG_CVAR( cg_enableBreath, "g_enableBreath", "0", CVAR_SERVERINFO, NULL )
CG_CVAR( cg_singlePlayerActive, "ui_singlePlayerActive", "0", CVAR_USERINFO, NULL )
CG_CVAR( cg_recordSPDemo, "ui_recordSPDemo", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_recordSPDemoName, "ui_recordSPDemoName", "", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_obeliskRespawnDelay, "g_obeliskRespawnDelay", "10", CVAR_SERVERINFO, NULL )
CG_CVAR( cg_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE, NULL )
#endif
CG_CVAR( cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT, NULL )
CG_CVAR( cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0, NULL )
CG_CVAR( cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0, NULL )
CG_CVAR( cg_timescale, "timescale", "1", 0, NULL )
CG_CVAR( cg_scorePlum, "cg_scorePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE, NULL )
CG_CVAR( cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE, NULL )
CG_CVAR( cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT, NULL )
CG_CVAR( cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_oldRail, "cg_oldRail", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_oldRocket, "cg_oldRocket", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_oldPlasma, "cg_oldPlasma", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_hitSounds, "cg_hitSounds", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_enemyModel, "cg_enemyModel", "", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_enemyColors, "cg_enemyColors", "", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_teamModel, "cg_teamModel", "", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_teamColors, "cg_teamColors", "", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_deadBodyDarken, "cg_deadBodyDarken", "1", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_fovAdjust, "cg_fovAdjust", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_followKiller, "cg_followKiller", "0", CVAR_ARCHIVE, NULL )
CG_CVAR( cg_killcam, "cg_killcam", "1", 0,
	"When 1, dying to another player shows a killcam: a replay of the last "
	"few seconds with the camera at the killer, aimed at you. Ends on its "
	"own shortly after the kill, or as soon as you respawn (clicking to "
	"respawn still works during the replay). "
	"When second bit is set (e.g. the value is 2), "
	"disables killcam when spectating." )
// Note that respawn delay after death is 1700 (see `respawnTime`).
// Let's also use the killcam transition as an indication
// that "you can respawn".
// But it overall feels good for it to be at ~1700.
// Though we want to make sure that when "Fire" is held
// then we don't transition to the killcam for a frame or two.
// Exactly 1700 would be better, but we seem to have some miscalculations.
CG_CVAR( cg_killcamStartDelay, "cg_killcamStartDelay", "1800", 0,
	"Death replay timing, all in milliseconds. The view switches "
	"cg_killcamStartDelay after dying; the replay covers from "
	"cg_killcamPreroll before the kill to cg_killcamPostroll after it. "
	"The replay history is limited by the snapshot ring buffer, "
	"(`KILLCAM_SNAPSHOT_BACKUP` times `cg_killcamRecordInterval` milliseconds): "
	"if it no longer holds the full preroll, "
	"the replay starts at the oldest recorded snapshot instead." )
CG_CVAR( cg_killcamStartOnClickDelay, "cg_killcamStartOnClickDelay", "9999999", 0,
	"After dying, freshly pressing attack (clicking) starts the death "
	"replay right away instead of waiting out cg_killcamStartDelay. "
	"Clicks within this many milliseconds of the death are ignored, as "
	"a grace period against accidental clicks; -1 disables starting on "
	"attack entirely. Note that only clicks within ~1.7 s of dying can "
	"start the replay: later ones make the server respawn us, ending "
	"the killcam as usual, so a second click still skips the replay. "
	"Disabled by default, because this might cause the player "
	"to respawn by mistake if they tried to click to start killcam "
	"but the respawn delay is already up." )
CG_CVAR( cg_killcamStartOnJumpDelay, "cg_killcamStartOnJumpDelay", "750", 0,
	"Same, for the jump key, which (unlike attack) never respawns us, so "
	"it can always start the replay early. The delay is just a grace "
	"period against jumps pressed around the moment of death; -1 disables "
	"starting on jump entirely." )
CG_CVAR( cg_killcamSkipOnJumpDelay, "cg_killcamSkipOnJumpDelay", "750", 0, NULL )
CG_CVAR( cg_killcamSkipPreRespawn, "cg_killcamSkipPreRespawn", "1", 0,
	"If `cg_killcamPreroll` starts at a time before we respawned, "
	"skip to the respawn" )
CG_CVAR( cg_killcamPreroll, "cg_killcamPreroll", "2500", 0, NULL )
CG_CVAR( cg_killcamPostroll, "cg_killcamPostroll", "2500", 0, NULL )
CG_CVAR( cg_killcamTimescale, "cg_killcamTimescale", "1", 0, NULL )
CG_CVAR( cg_killcamTimescaleBefore, "cg_killcamTimescaleBefore", "20", 0,
	"How much of the replay runs at cg_killcamTimescale, in milliseconds "
	"before and after the kill." )
CG_CVAR( cg_killcamTimescaleAfter, "cg_killcamTimescaleAfter", "20", 0, NULL )
CG_CVAR( cg_killcamTimescaleFadeIn, "cg_killcamTimescaleFadeIn", "50", 0,
	"How long to ease into cg_killcamTimescale before that window and "
	"back out to real time after it, in milliseconds. The fades sit "
	"outside the window, so the window itself still runs at the full "
	"timescale. 0 = change speed instantly." )
CG_CVAR( cg_killcamTimescaleFadeOut, "cg_killcamTimescaleFadeOut", "400", 0, NULL )
CG_CVAR( cg_killcamRecordInterval, "cg_killcamRecordInterval", "20", 0,
	"How often to record snapshots for the killcam, in milliseconds: "
	"if we get a snapshot sooner than this since the last recorded one, "
	"we'll skip it. "
	"The resulting possible killcam duration will be "
	"this times `KILLCAM_SNAPSHOT_BACKUP`. "
	"See also `snaps` and `sv_fps` CVARs. "
	"TODO we probably should determine this dynamically, "
	"based on the total killcam duration (the CVARs above)? "
	"And maybe this should not be needed at all, "
	"as we should simply always have a big enough buffer. "
	"But I guess it's fun to know that we can just drop some snapshots "
	"and things will still work fine." )
CG_CVAR( cg_killcamSuicides, "cg_killcamSuicides", "1", 0,
	"When 1, suicides and world deaths (lava, falling, ...) also get a "
	"killcam: a replay of ourselves (own first-person view while still "
	"alive, third person after the death). When 0, only deaths caused by "
	"other players do (also see cg_killcamLastAttacker)." )
CG_CVAR( cg_killcamLastAttacker, "cg_killcamLastAttacker", "7500", 0,
	"For suicides and world deaths: if another player damaged us within "
	"this many milliseconds before the death (e.g. knocked us off a "
	"ledge), show the killcam from their side instead. Works regardless "
	"of cg_killcamSuicides. 0 = off." )
CG_CVAR( cg_killcamFirstPerson, "cg_killcamFirstPerson", "0", 0,
	"When 1, the death replay is shown from the killer's eyes (with their "
	"view weapon), like a classic killcam. When 0, a third-person camera "
	"floats behind the killer (see the placement cvars below). Falls back "
	"to third person when the killer is dead or not in the recorded data. "
	"TODO kinda experimental. For example, there is a bug "
	"where there are visibly 2 lightning beams." )
CG_CVAR( cg_killcamRange, "cg_killcamRange", "50", 0,
	"Killcam camera placement: how far behind, above and to the side of "
	"the killer's head the camera floats. The height and side offsets keep "
	"the killer's model and the award icons above their head from covering "
	"the victim at the center of the screen. Positive side = camera to the "
	"killer's right (the killer appears left of center), negative = left." )
CG_CVAR( cg_killcamHeight, "cg_killcamHeight", "24", 0, NULL )
CG_CVAR( cg_killcamSide, "cg_killcamSide", "-15", 0, NULL )
#ifndef KILLCAM_NO_MISSILE_CHASE
CG_CVAR( cg_killcamMissile, "cg_killcamMissile", "1", 0,
	"When the kill was scored with a slow missile (rocket, grenade, BFG), "
	"chase that missile with the camera from launch to explosion, then "
	"watch the victim from the explosion point for the rest of the "
	"replay. Plasma is deliberately not followed (cells are a fast "
	"stream; chasing one for a fraction of a second is jarring)." )
CG_CVAR( cg_killcamMissileMinDuration, "cg_killcamMissileMinDuration", "300", 0,
	"Don't switch to the missile-chase camera if the missile's recorded "
	"flight (first sighting to explosion) is shorter than this many "
	"milliseconds -- e.g. a point-blank rocket; the killer camera is "
	"kept instead. 0 = always chase." )
CG_CVAR( cg_killcamMissileRange, "cg_killcamMissileRange", "", 0,
	"How far behind / above / to the side of the missile the chase camera "
	"floats. When empty (the default), these are derived from where the "
	"camera already is at the moment the chase begins, so the cut to the "
	"missile camera doesn't make the camera jump. Set a number to "
	"override an axis. The side convention matches cg_killcamSide "
	"(positive = to the right of the flight direction)." )
CG_CVAR( cg_killcamMissileHeight, "cg_killcamMissileHeight", "", 0, NULL )
CG_CVAR( cg_killcamMissileSide, "cg_killcamMissileSide", "", 0, NULL )
CG_CVAR( cg_killcamMissileLookAtTarget, "cg_killcamMissileLookAtTarget", "1", 0,
	"Where the missile-chase camera looks: "
	"0 = along the missile's flight direction; "
	"1 = at the target (the victim); "
	"2 = at the target for grenades (whose lobbed arcs rarely point at "
	"the victim), along the flight direction for the rest." )
#endif // KILLCAM_NO_MISSILE_CHASE
CG_CVAR( cg_killcamHitSounds, "cg_killcamHitSounds", "0", 0, NULL )
CG_CVAR( cg_killcamTest, "cg_killcamTest", "0", 0,
	"Killcam development / testing: when > 0, render the world this many "
	"milliseconds in the past (from your own point of view), replayed from "
	"recorded snapshots in the killcam context. 0 = off." )

#undef CG_CVAR
