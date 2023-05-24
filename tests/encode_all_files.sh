for file in *; do
  if [ ${file: -4} == ".asm" ]
  then 
    file_to_write="${file::-4}.x2017"
    python3 asm_to_hex.py $file $file_to_write
  fi
done
