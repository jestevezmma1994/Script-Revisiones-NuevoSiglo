for i in `find -name '*.dot'`; do
    echo $i
    ./generate.sh $1 $i &
done

wait

if [ ! -h "$1/index.html" ]; then
    ln -s CubiTV.html $1/index.html
fi
