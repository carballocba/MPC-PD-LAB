# Fresh-install-only MPUL guard. Runs before staging/settings/service mutations.
[ ! -e "$SYNTHS/$SKIN" ] && [ ! -L "$SYNTHS/$SKIN" ] || die "Universal folder already exists; refusing overwrite"
[ ! -e "$SYNTHS/.$SKIN.new" ] && [ ! -L "$SYNTHS/.$SKIN.new" ] || die "Universal staging folder already exists"
[ ! -e "$SYNTHS/.$SKIN.old" ] && [ ! -L "$SYNTHS/.$SKIN.old" ] || die "Universal backup folder already exists"
[ ! -e "$OLDFILE" ] && [ ! -L "$OLDFILE" ] || die "legacy Universal binary exists; refusing migration"
if grep -i -E 'uid="0*4d50554c"|mpc_pd_universal_loader\.so' "$SETTINGS" >/dev/null; then
    die "MPUL UID or Universal binary already registered; resolve collision before installing"
fi
