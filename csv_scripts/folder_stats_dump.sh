for f in `ls $1`;do
    python dump_stats.py $1/$f;
done
