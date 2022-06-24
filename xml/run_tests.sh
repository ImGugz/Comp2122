#which bin   
#which fritzing
#!/usr/bin/
for file in ../auto-tests/*; do
 if [ ! -d "$file" ]; then
        filename=$(basename -- "$file")
        extension="${filename##*.}"
        filename="${filename%.*}"
        ../l22 ../auto-tests/$filename.l22 -o output/$filename.xml.xml > /dev/null 2>&1
  fi
done

for file in ../auto-tests/*; do
  if [ ! -d "$file" ]; then
        filename=$(basename -- "$file")
        extension="${filename##*.}"
        filename="${filename%.*}"
        if ! cmp -s "output/$filename.xml.xml" "expected/$filename.xml.xml" ; then
          diff output/$filename.xml.xml expected/$filename.xml.xml
          echo "$filename is not ok!"
        fi
  fi


done

