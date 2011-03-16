#!/bin/sh

if test -z "$1" -o ! -e "$1"
then
	echo "Usage: $0 <file.dot>"
	exit 1
fi

OUTPUTDIR="testlayouts"
OUTPUTHTML="${OUTPUTDIR}/index.html"

mkdir -p ${OUTPUTDIR}

cat <<EOH > ${OUTPUTHTML}
<html>
<body>
EOH

for f in dot neato twopi circo fdp sfdp osage patchwork
do 
 echo $f
 dot -K$f -Tpng -o${OUTPUTDIR}/$f.png $1
 echo "${f}: <img src=\"$f.png\" width=400>" >> ${OUTPUTHTML}
done

cat <<EOH >> ${OUTPUTHTML}
</body>
</html>
EOH

echo "Now open ${OUTPUTHTML}"

