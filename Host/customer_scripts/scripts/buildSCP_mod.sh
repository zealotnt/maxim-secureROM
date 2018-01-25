#!/bin/sh

scriptsDir=$(pwd)

# Parameters
verbose=no
soc=

while [ $# -ge 1 ]; do
	case $1 in
	--soc=)	echo >&2 "error: No ROM/chip version specified"
		exit 4
		;;
	--soc=*)
		soc=${1#--soc=}
		;;
	--verbose)
		verbose=no
		;;
	--)	shift
		break
		;;
	-*)	echo >&2 "error: Unknown option \`$1'"
		exit 5
		;;
	*)	break
		;;
	esac
	shift
done
readonly soc verbose

case $# in
3)	readonly input=$scriptsDir/$1
	output=$2
	readonly outputFileName=$2
	readonly key=$scriptsDir/../keys/maximtestcrk.key
	dest_folder=$3
	;;
*)	usage >&2
	exit 2
	;;
esac

TOOLDIR=$(readlink -e -- "$(dirname -- "$0")")
INPUTFILE=$(readlink -e -- "$input")  || {
	echo >&2 "error: Input \`$input' does not exist"
	exit 1
}
KEYFILE=$(readlink -e -- "$key")  || {
	echo >&2 "error: Private key \`$key' does not exist"
	exit 3
}

#remove end slash if any
output=${scriptsDir%/}/${output%/}

rm -rf -- "$output"
mkdir  -- "$output"

OUTPUTDIR=$(readlink -e -- "$output")

cd -- "$output"   ||  exit 10

built_for=$( od -An -j8 -N4 -tx1 -- "$INPUTFILE" )  || {
	echo >&2 "error: Cannot determine ROM version from \`$INPUTFILE'"
	exit 20
}
readonly bin_version=$( echo $built_for | tr -d ' ' )

# If an explicit chip/ROM (--soc=...) was specified, use that
# (warning if if seems to differ from what we extracted from the input);
# Otherwise (by default), use whatever ROM version we extracted from the input.
if [ -z "$soc" ]; then
	ROMVERSION=$bin_version
elif use_rom=$( $TOOLDIR/../lib/rom/findrom.sh "$soc" ); then
	ROMVERSION=${use_rom##*/}
else
	exit 21
fi
readonly ROMVERSION
[ "X$ROMVERSION" = "X$bin_version" ]  || {
	echo >&2 "\
warning: Input built for ROM $bin_version not $ROMVERSION: \`$INPUTFILE'"
}

echo "ROMVERSION=$ROMVERSION"
romdir=$( $TOOLDIR/../lib/rom/findrom.sh "$ROMVERSION" )  ||  exit 22
readonly romdir

which objcopy >/dev/null  || {
	echo >&2 "\
error: \`objcopy' utility not available.
   You should install the \"binutils\" package."
	exit 30
}

flash_dest=0x10000000

objcopy -I binary -O srec --srec-forceS3 --srec-len=128 --adjust-vma $flash_dest $INPUTFILE binary.s19  || {
	echo >&2 "ERROR."
	exit 31
}

cat >> sb_script.txt << EOF
write-file binary.s19
EOF

$TOOLDIR/../lib/session_build.exe session_mode=SCP_ANGELA_ECDSA verbose=no output_file=$OUTPUTDIR/scp pp=ECDSA addr_offset=00000000 chunk_size=4094 script_file=sb_script.txt ecdsa_file=$KEYFILE  &&
	LC_ALL=C  ls -1 *.packet >packet.list

# Rename output SCP folder into "tempFw"
mkdir -p ${scriptsDir%/}/tempFw
cp -rp $output ${scriptsDir%/}/tempFw
cd ${scriptsDir%/}/tempFw
echo cd ${scriptsDir%/}/tempFw
tar -cf $outputFileName.tar $outputFileName
echo tar -cf $outputFileName.tar $outputFileName
# Clean generated file/folder
rm $input
# rm -rf tempFw

# Copy tar file to user's current directory
cp $outputFileName.tar $dest_folder
echo cp $outputFileName.tar $dest_folder
echo "$outputFileName.tar file created"
sleep 1


if [ $? -ne 0 ] ; then
echo "ERROR."
exit 1
fi

