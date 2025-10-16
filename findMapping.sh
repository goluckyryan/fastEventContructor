#!/bin/bash

output_file="GS_channel_map.txt"

# Write timestamp and header to the file
{
  echo "Generated on: $(date '+%Y-%m-%d %H:%M:%S')"
  printf "%-10s %-8s %-8s %-8s %-8s\n" "GS number" "VME" "DIG" "Channel" "BoardID"
} > "$output_file"

# Loop from 001 to 110
for detnum in $(seq -w 1 110); do
  gs_name="GS${detnum}"
  # Fetch values with caget
  vme=$(caget ${gs_name}_VME_Index | awk '{print $NF}')
  dig=$(caget ${gs_name}_Dig_Index | awk '{print $NF}')
  chan=$(caget ${gs_name}_Dig_Channel | awk '{print $NF}')
  
  if [ "$vme" == "-1" ]; then
    BoardID="-1"
  else
    VME=$(printf "%02d" "$vme")
    BoardID=$(caget "VME${VME}:MDIG${dig}:user_package_data_RBV" | awk '{print $NF}')
  fi

  echo "Processing $gs_name: VME=$vme, DIG=$dig, Channel=$chan, BoardID=$BoardID"

  # Append to the output file
  printf "%-10s %-8s %-8s %-8s %-8s\n" "$detnum" "$vme" "$dig" "$chan" "$BoardID">> "$output_file"
done