FOLDER=$1
if [ "$FOLDER" == "" ]; then
  echo "Usage: $0 <folder>"
  exit
fi

for HAND in hours minutes; do
  ANGLE=0
  convert data/$FOLDER/${HAND}Hand_$ANGLE.bmp -threshold 1% -colors 2 data/$FOLDER/${HAND}Hand_mask_$ANGLE.bmp
  ANGLE=6
  while [ $ANGLE -lt 90 ]; do
    echo "Angle: $ANGLE"
    convert data/$FOLDER/${HAND}Hand_0.bmp -background black -define bmp:subtype=RGB565 -rotate $ANGLE data/$FOLDER/${HAND}Hand_$ANGLE.bmp
    convert data/$FOLDER/${HAND}Hand_$ANGLE.bmp -threshold 1% -colors 2 -background black data/$FOLDER/${HAND}Hand_mask_$ANGLE.bmp
    let ANGLE=ANGLE+6
  done
done
