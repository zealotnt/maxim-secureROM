#!/bin/bash
#
# findrom.sh	-- Find the SecureROM stuff for a specific ROM version
#
# $1 (mandatory) is the ROM to find, either a version (hex),
#    or a commonly-used chip name (e.g., B2).
#
# $2, $3, ... (optional) are the directories to search;
#             default is the directory implied by $0.
#

readonly pathScript=$0
readonly baseScript=${pathScript##*/}

case $# in
0)	echo >&2 -E "Usage: $baseScript ROM [DIR]..."
	exit 2
	;;
1)	rom=$1
	set -- "$(dirname -- "$pathScript")"
	;;
*)	rom=$1
	shift
	;;
esac
readonly rom

# $rom names cannot be empty or contain slashes.
# If the $rom name is exactly two characters, or contains a non-hex digit,
# then it cannot be a ROM version and hence must be a chip name.
# Otherwise, when the $rom name is only hex digits (optionally prefixed
# with either `0x' or `0X'), then it must be a ROM version.
#
case $rom in
"" | */*)
	echo  >&2  -E "$baseScript: error: Impossible ROM/chip name \`$rom'"
	exit 3
	;;
0[xX]?*)
	could_be_version=true
	;;
?? | *[^[:xdigit:]]*)
	could_be_version=false
	;;
*)	could_be_version=true
	;;
esac
readonly could_be_version

# Is $1 a valid directory ?
# Valid directories contain at least the four non-empty files:
#
#	names . . . . . . . . . . Chip names, one per line, cAsE-sensitive
#	scpa_nand_applet.s19  . . SCP NAND programming Applet
#	scpa_nand_script.txt  . . Associated script
#
is_romdir() {
	local d="${1:-.}"

	[ -d "$d" ]				||  return 1
	[ -s "$d/names" ]			||  return 2

	return 0
}

# Is $1 the name of a chip for the ROM described by directory $2 ?
#
is_chipname() {
	local d="${2:?}"

	is_romdir "$d"  &&  awk -vROM="${1:?}" -- '
BEGIN		{ sts = 1	}
NF == 0 || /^#/	{ next		}
$1 == ROM	{ sts = 0; exit	}
END		{ exit sts	}' "$d/names"
}

# Convert input hex number into an 8-digit hex number
# with no "0x" but with as many leading "0"s as needed.
# The hex input ($1) may optionally start with "0x" and
# does not have to be exactly 8-digits.
# Call as:
#
#   new_value=$( fixup_hex "$old_value" ) || echo error ...
#
# Return status is non-0 if there is a problem.
#
fixup_hex() {
	case ${1:?} in
	0[xX]*)	printf "%08x" "$1"      ;;
	*)	printf "%08x" "0x$1"    ;;
	esac
}

# Is $rom a ROM version, i.e., does $rom name a (valid) directory
# in one of the directories-to-search?
#
${could_be_version:?}  &&  vers=$( fixup_hex "$rom"  2>/dev/null )  && {
	for d in "$@"; do
		is_romdir "$d/${vers:?}"  &&  {
			echo -E "$d/$vers"
			exit 0
		}
	done
}

# No, so is $rom a common name for one of the chips?
#
for d in "$@"; do
	for s in "$d"/*; do
		is_chipname "$rom" "$s"  && {
			echo -E "$s"
			exit 0
		}
	done
done

# No, so we have no idea what $rom is!
#
echo  >&2  -E "$baseScript: error: Unknown ROM/chip \`$rom'"
exit 1
