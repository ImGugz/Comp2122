#which bin   
#which fritzing
#!/usr/bin/

for file in auto-tests/*; do
  if [ ! -d "$file" ]; then
        filename=$(basename -- "$file")
        extension="${filename##*.}"
        filename="${filename%.*}"
        ./l22 auto-tests/$filename.l22 -o xml_output/$filename.xml.xml
  fi
done

clear

for file in auto-tests/*; do
  if [ ! -d "$file" ]; then
        echo "File $file"
        echo
        filename=$(basename -- "$file")
        extension="${filename##*.}"
        filename="${filename%.*}"
        cat auto-tests/$filename.l22 
        echo
        echo "Result:"
        echo
        cat xml_output/$filename.xml.xml
  fi
  echo 
  echo
  echo "*---------------------------------*"
  echo
  echo
done

