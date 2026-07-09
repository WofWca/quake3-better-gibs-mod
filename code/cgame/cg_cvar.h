#ifdef EXTERN_CG_CVAR
	#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) extern vmCvar_t vmCvar;
#endif

#ifdef DECLARE_CG_CVAR
	#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) vmCvar_t vmCvar;
#endif

#ifdef CG_CVAR_LIST
	#define CG_CVAR( vmCvar, cvarName, defaultString, cvarFlags ) { & vmCvar, cvarName, defaultString, cvarFlags },
#endif

CG_CVAR( cg_ignore, "cg_ignore", "0", 0 ) // used for debugging
CG_CVAR( cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE )
CG_CVAR( cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE )
CG_CVAR( cg_fov, "cg_fov", "90", CVAR_ARCHIVE )
CG_CVAR( cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE )
CG_CVAR( cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE )
CG_CVAR( cg_gibs, "cg_gibs", "1.0", CVAR_ARCHIVE )
CG_CVAR( cg_oldGibs, "cg_oldGibs", "0", CVAR_ARCHIVE )
CG_CVAR( cg_gibsInheritPlayerVelocity, "cg_gibsInheritPlayerVelocity", "1.0", CVAR_ARCHIVE )
// On vanilla servers this only has effect on random and vertical velocity
// but not on directional velocity.
// Remember that this also affects own gibs, so having a value too high
// will make gibs fly faster than the camera.
CG_CVAR( cg_gibsKnockback, "cg_gibsKnockback", "1.0", 0 )
CG_CVAR( cg_gibsExtraKnockback, "cg_gibsExtraKnockback", "0", 0 )
// For every 100 of knockback speed above 500 (100 damage),
// add this many pieces of gibs, times the value of `cg_gibs`.
//
// With higher damage gibs fly further apart, so one starts to see gibs
// as individual pieces instead of a "cloud", which IMO is not good.
// Increasing the amount of gibs with damage fixes this.
// And overall it makes it more rewarding to deal more damage.
// Q: "What sense does it make to have more gibs?
// Did the player got bigger from taking more damage??":
// A: No, they simlpy got split into more pieces.
CG_CVAR( cg_gibsPiecesFromKnockback, "cg_gibsPiecesFromKnockback", "1.1", CVAR_ARCHIVE )
CG_CVAR( cg_gibsLinearVelocityFromKnockback, "cg_gibsLinearVelocityFromKnockback", "1.0", 0 )
CG_CVAR( cg_gibsRandomVelocityFromKnockback, "cg_gibsRandomVelocityFromKnockback", "0.4", CVAR_ARCHIVE )
CG_CVAR( cg_gibsVerticalVelocityFromKnockback, "cg_gibsVerticalVelocityFromKnockback", "0.2", CVAR_ARCHIVE )
CG_CVAR( cg_gibsExtraRandomVelocity, "cg_gibsExtraRandomVelocity", "100", CVAR_ARCHIVE )
CG_CVAR( cg_gibsExtraVerticalVelocity, "cg_gibsExtraVerticalVelocity", "50", CVAR_ARCHIVE )
// How much to slow down the player depending on how much damage (knockback)
// they took.
// This makes the original player velocity have less effect
// on the final gib velocity, thus giving more emphasis to the damage direction.
//
// Unlike reducing `cg_gibsInheritPlayerVelocity`,
// this works better if the player is already moving at high speed
// (e.g. on a jump pad), or falling.
// This will not reduce player velocity if the knockback was low
// or if damage had no direction (e.g. falling).
//
// Has no effect on vanilla servers.
CG_CVAR( cg_gibsPlayerSpeedFromKnockback, "cg_gibsPlayerSpeedFromKnockback", "-0.25", 0 )
// If knockback is big, this helps keep at least some fraction of player speed.
// For example, if this is -0.75, we'll always keep at least 25%
// of original player velocity.
CG_CVAR( cg_gibsPlayerSpeedFromKnockbackMaxFraction, "cg_gibsPlayerSpeedFromKnockbackMaxFraction", "-0.5", 0 )
CG_CVAR( cg_gibsDuration, "cg_gibsDuration", "5000", 0 )
// Whether to use the exact position where the player got gibbed
// as the initial gibs position, instead of the position of the gibbed player
// on the screen, i.e. the position of the player on the next server frame
// (lerpOrigin).
//
// Can be a fractional value between 0 and 1,
// but you probably want either 0 or 1.
// To get vanilla behavior, set to 0.
//
// Has no effect on vanilla servers (`g_gibsNewEvGibPlayerProtocol 0`).
// Basically has no effect when `cg_gibsNoLerpDelay 1`.
//
// Using the original position might make it seem
// like the player "telepoted" one snapshot back (50ms at `snaps 20`)
// if they're already movnig at high speed.
CG_CVAR( cg_gibsOriginalOrigin, "cg_gibsOriginalOrigin", "0.0", 0 )
// Don't wait for next snapshot to arrive (normally 50ms)
// before gibbing players.
// Consequently, this affects the position from which the gibs are launched,
// due to the gibbed player (or, rather, their camera) moving during those 50ms.
// See `cg_gibsOriginalOrigin`.
//
// Note that when looking at another player (or missile) gibbing someone
// it will look like the player gets gibbed
// 50ms before the actual shot is fired.
// That is, the gib even will happen ahead of the displayed state of the game.
//
// Set this to 3 (2nd bit) to also do this on vanilla servers.
// To get vanilla behavior, set to 0.
CG_CVAR( cg_gibsNoLerpDelay, "cg_gibsNoLerpDelay", "0", 0 )
CG_CVAR( cg_gibsBounceFactor, "cg_gibsBounceFactor", "0.4", CVAR_ARCHIVE )
CG_CVAR( cg_gibsBounceFactorRandomness, "cg_gibsBounceFactorRandomness", "0.5", CVAR_ARCHIVE )
CG_CVAR( cg_gibsRotationFactor, "cg_gibsRotationFactor", "1.0", CVAR_ARCHIVE )
// Not archived because experimental
CG_CVAR( cg_gibsBloodTrailPeriod, "cg_gibsBloodTrailPeriod", "150", 0 )
CG_CVAR( cg_gibsFireTrailPeriod, "cg_gibsFireTrailPeriod", "30", 0 )
CG_CVAR( cg_gibsFireTrailDuration, "cg_gibsFireTrailDuration", "1024", 0 )
CG_CVAR( cg_gibsFireTrailMinKnockback, "cg_gibsFireTrailMinKnockback", "-1", 0 )
CG_CVAR( cg_gibsBetterCameraOnGib, "cg_gibsBetterCameraOnGib", "1", CVAR_USERINFO | CVAR_ARCHIVE )
CG_CVAR( cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE )
CG_CVAR( cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE )
CG_CVAR( cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  )
CG_CVAR( cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawSpeed, "cg_drawSpeed", "0", CVAR_ARCHIVE )
CG_CVAR( cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE )
CG_CVAR( cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE )
CG_CVAR( cg_drawWeaponSelect, "cg_drawWeaponSelect", "1", CVAR_ARCHIVE )
CG_CVAR( cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE )
CG_CVAR( cg_crosshairHealth, "cg_crosshairHealth", "1", CVAR_ARCHIVE )
CG_CVAR( cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE )
CG_CVAR( cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE )
CG_CVAR( cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE )
CG_CVAR( cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE )
CG_CVAR( cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE )
// Note that ~290 corresponds to a free fall with no bounce from player height.
CG_CVAR( cg_bounceMarksMinImpactSpeed, "cg_bounceMarksMinImpactSpeed", "200", CVAR_ARCHIVE )
CG_CVAR( cg_bounceSoundMinImpactSpeed, "cg_bounceSoundMinImpactSpeed", "450", CVAR_ARCHIVE )
CG_CVAR( cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE )
CG_CVAR( cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE  )
CG_CVAR( cg_railTrailRadius, "cg_railTrailRadius", "0", CVAR_ARCHIVE  )
CG_CVAR( cg_gun_frame, "cg_gun_frame", "", CVAR_ROM )
CG_CVAR( cg_gun_x, "cg_gunX", "0", CVAR_ARCHIVE )
CG_CVAR( cg_gun_y, "cg_gunY", "0", CVAR_ARCHIVE )
CG_CVAR( cg_gun_z, "cg_gunZ", "0", CVAR_ARCHIVE )
CG_CVAR( cg_centertime, "cg_centertime", "3", CVAR_CHEAT )
CG_CVAR( cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE )
CG_CVAR( cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE )
CG_CVAR( cg_bobup , "cg_bobup", "0.005", CVAR_ARCHIVE )
CG_CVAR( cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE )
CG_CVAR( cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE )
CG_CVAR( cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT )
CG_CVAR( cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT )
CG_CVAR( cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT )
CG_CVAR( cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT )
CG_CVAR( cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT )
CG_CVAR( cg_debugGibs, "cg_debugGibs", "0", CVAR_CHEAT )
CG_CVAR( cg_errorDecay, "cg_errordecay", "100", 0 )
CG_CVAR( cg_nopredict, "cg_nopredict", "0", 0 )
CG_CVAR( cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT )
CG_CVAR( cg_showmiss, "cg_showmiss", "0", 0 )
CG_CVAR( cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT )
CG_CVAR( cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT )
CG_CVAR( cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT )
CG_CVAR( cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT )
CG_CVAR( cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT )
CG_CVAR( cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT )
CG_CVAR( cg_thirdPerson, "cg_thirdPerson", "0", 0 )
CG_CVAR( cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE  )
CG_CVAR( cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE  )
CG_CVAR( cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  )
CG_CVAR( cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE )
#ifdef MISSIONPACK
CG_CVAR( cg_deferPlayers, "cg_deferPlayers", "0", CVAR_ARCHIVE )
#else
CG_CVAR( cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE )
#endif
CG_CVAR( cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE )
CG_CVAR( cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO )
CG_CVAR( cg_stats, "cg_stats", "0", 0 )
CG_CVAR( cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE )
CG_CVAR( cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE )
#ifdef MISSIONPACK
CG_CVAR( cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE )
CG_CVAR( cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE )
#endif
// the following variables are created in other parts of the system,
// but we also reference them here
CG_CVAR( cg_buildScript, "com_buildScript", "0", 0 )	// force loading of all possible data amd error on failures
CG_CVAR( cg_paused, "cl_paused", "0", CVAR_ROM )
CG_CVAR( cg_blood, "com_blood", "1", CVAR_ARCHIVE )
#ifdef MISSIONPACK
CG_CVAR( cg_redTeamName, "g_redteam", DEFAULT_REDTEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO )
CG_CVAR( cg_blueTeamName, "g_blueteam", DEFAULT_BLUETEAM_NAME, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO )
CG_CVAR( cg_currentSelectedPlayer, "cg_currentSelectedPlayer", "0", CVAR_ARCHIVE )
CG_CVAR( cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName", "", CVAR_ARCHIVE )
CG_CVAR( cg_singlePlayer, "ui_singlePlayerActive", "0", CVAR_USERINFO )
CG_CVAR( cg_enableDust, "g_enableDust", "0", CVAR_SERVERINFO )
CG_CVAR( cg_enableBreath, "g_enableBreath", "0", CVAR_SERVERINFO )
CG_CVAR( cg_singlePlayerActive, "ui_singlePlayerActive", "0", CVAR_USERINFO )
CG_CVAR( cg_recordSPDemo, "ui_recordSPDemo", "0", CVAR_ARCHIVE )
CG_CVAR( cg_recordSPDemoName, "ui_recordSPDemoName", "", CVAR_ARCHIVE )
CG_CVAR( cg_obeliskRespawnDelay, "g_obeliskRespawnDelay", "10", CVAR_SERVERINFO )
CG_CVAR( cg_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE )
#endif
CG_CVAR( cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT )
CG_CVAR( cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE )
CG_CVAR( cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0 )
CG_CVAR( cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0 )
CG_CVAR( cg_timescale, "timescale", "1", 0 )
CG_CVAR( cg_scorePlum, "cg_scorePlums", "1", CVAR_USERINFO | CVAR_ARCHIVE )
CG_CVAR( cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE )
CG_CVAR( cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT )
CG_CVAR( cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE )
CG_CVAR( cg_noProjectileTrail, "cg_noProjectileTrail", "0", CVAR_ARCHIVE )
CG_CVAR( cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE )
CG_CVAR( cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE )
CG_CVAR( cg_oldRail, "cg_oldRail", "1", CVAR_ARCHIVE )
CG_CVAR( cg_oldRocket, "cg_oldRocket", "1", CVAR_ARCHIVE )
CG_CVAR( cg_oldPlasma, "cg_oldPlasma", "1", CVAR_ARCHIVE )
CG_CVAR( cg_trueLightning, "cg_trueLightning", "0.0", CVAR_ARCHIVE )
CG_CVAR( cg_hitSounds, "cg_hitSounds", "0", CVAR_ARCHIVE )
CG_CVAR( cg_enemyModel, "cg_enemyModel", "", CVAR_ARCHIVE )
CG_CVAR( cg_enemyColors, "cg_enemyColors", "", CVAR_ARCHIVE )
CG_CVAR( cg_teamModel, "cg_teamModel", "", CVAR_ARCHIVE )
CG_CVAR( cg_teamColors, "cg_teamColors", "", CVAR_ARCHIVE )
CG_CVAR( cg_deadBodyDarken, "cg_deadBodyDarken", "1", CVAR_ARCHIVE )
CG_CVAR( cg_fovAdjust, "cg_fovAdjust", "0", CVAR_ARCHIVE )
CG_CVAR( cg_followKiller, "cg_followKiller", "0", CVAR_ARCHIVE )

#undef CG_CVAR
