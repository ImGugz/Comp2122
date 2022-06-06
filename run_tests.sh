#which bin   
#which fritzing
#!/usr/bin/

for file in auto-tests/*; do
  if [ ! -d "$file" ]; then
        filename=$(basename -- "$file")
        extension="${filename##*.}"
        filename="${filename%.*}"
        ./l22 auto-tests/$filename.l22 -o xml/expected/$filename.xml.xml > output.log
  fi
done

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
        cat xml/expected/$filename.xml.xml
  fi
  echo 
  echo
  echo "*---------------------------------*"
  echo
  echo
done

clear

for file in auto-tests/*; do
 if [ ! -d "$file" ]; then
      if [ ! -s xml/expected/$filename.xml.xml ]; then
        echo $filename is empty ...
      fi
fi
  
done



