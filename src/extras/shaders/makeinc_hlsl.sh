#!sh
cd obj
for i in *cso; do
	(echo -n 'static '
	xxd -i $i | grep -v '_len = ') > ${i%cso}inc
done
