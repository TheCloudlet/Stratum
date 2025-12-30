#!/bin/bash
# Convert Valgrind lackey trace to Stratum format
# Usage: ./convert_lackey.sh lackey.log output.txt
#    or: valgrind --tool=lackey --trace-mem=yes ./program 2>&1 | ./convert_lackey.sh > output.txt

if [ $# -eq 2 ]; then
    # File mode: read from file, write to file
    perl -ne 'BEGIN{print "# Type  Addr\n"} 
      if(/^ ([LSM])\s+0x([0-9a-fA-F]+)/){
        $addr=hex($2);
        $aligned=int($addr/64)*64;
        $type=($1 eq "M")?"S":$1;
        printf "%s       0x%X\n",$type,$aligned
      }' "$1" > "$2"
    echo "Converted $1 -> $2" >&2
elif [ $# -eq 0 ]; then
    # Pipe mode: read from stdin, write to stdout
    perl -ne 'BEGIN{print "# Type  Addr\n"} 
      if(/^ ([LSM])\s+0x([0-9a-fA-F]+)/){
        $addr=hex($2);
        $aligned=int($addr/64)*64;
        $type=($1 eq "M")?"S":$1;
        printf "%s       0x%X\n",$type,$aligned
      }'
else
    echo "Usage:" >&2
    echo "  $0 lackey.log output.txt        # Convert file" >&2
    echo "  valgrind ... | $0 > output.txt  # Convert from pipe" >&2
    exit 1
fi
