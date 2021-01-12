mkdir -p $1/`dirname $2`
f=${2%%.dot}
f=${f##./}
fbase=`basename $f`
dot -Tpng -o$1/$f.png $f.dot -Tcmapx -o$1/$f.cmapx
echo "<img src=\"$fbase.png\" usemap=\"#$fbase\" />" > $1/$f.html
cat $1/$f.cmapx >> $1/$f.html
