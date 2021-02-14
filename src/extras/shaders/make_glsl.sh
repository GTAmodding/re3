#!sh
for i in *.vert; do
	echo $i
	./makeinc_glsl.sh $i
done
for i in *.frag; do
	echo $i
	./makeinc_glsl.sh $i
done
