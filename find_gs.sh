#!/usr/bin/env bash
# scripts/find_gs.sh
# Lookup GS number in GS_channel_map.txt and print VME, DIG, Channel, BoardID

set -euo pipefail
IFS=$'\n\t'

CHANMAP="GS_channel_map.txt"

function usage(){
  cat <<EOF
Usage: $0 <GS_number>
Example:
  $0 15     # same as 015
  $0 015

Outputs columns: VME DIG Channel BoardID

EOF
}

if [[ ${#@} -lt 1 || "$1" == "-h" || "$1" == "--help" ]]; then
  usage
  exit 1
fi

if [[ ! -f "$CHANMAP" ]]; then
  echo "Channel map file '$CHANMAP' not found in current directory." >&2
  exit 2
fi

input="$1"
# normalize to 3-digit with leading zeros if numeric
if [[ "$input" =~ ^[0-9]+$ ]]; then
  gs=$(printf "%03d" "$input")
else
  gs="$input"
fi

# Print header
printf "#################################### BGO\n"
printf "GS_number  VME  DIG  Channel  BoardID\n"

# Search (skip first 2 lines)
awk -v g="$gs" 'BEGIN{found=0} NR>2 { if($1==g){ printf "%s %6s %4s %8s %8s\n", $1, $2, $3, $4, $5; found=1 } } END{ if(found==0) exit 1 }' "$CHANMAP" || {
  echo "GS number '$gs' not found." >&2
  exit 3
}

exit 0
