# Fresh-install-only bridge release: refuse all upstream replacement/migration.
[ ! -e "$SYNTHS/$SKIN" ] && [ ! -L "$SYNTHS/$SKIN" ] || die "plugin folder already exists"
[ ! -e "$SYNTHS/.$SKIN.new" ] && [ ! -L "$SYNTHS/.$SKIN.new" ] || die "staging folder already exists"
[ ! -e "$SYNTHS/.$SKIN.old" ] && [ ! -L "$SYNTHS/.$SKIN.old" ] || die "backup folder already exists"
[ ! -e "$OLDFILE" ] && [ ! -L "$OLDFILE" ] || die "legacy plugin exists; migration refused"
if grep -i -E "uid=\"0*$UID_HEX\"" "$SETTINGS" >/dev/null || grep -F "$SO" "$SETTINGS" >/dev/null; then
    die "UID or binary already registered; collision/upgrade refused"
fi
