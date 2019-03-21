printf "\ntesting parallel capabilities of the server (running 16 sleep handlers)\n"
processes=()
tooslow=0
for i in {1..4} ; do
	curl ss.gitrdone.cs130.org/echo &
	processes+=($!)
done
sleep 5

for i in "${processes[@]}"; do
	if ps -p $i > /dev/null
	then
		echo "  !!server is too slow, server isn't running in multithreaded"
		kill $i
		tooslow=1
	fi
done
if [ $tooslow -ne 1 ] ; then
	echo "  --server is running multithreaded\n"
fi