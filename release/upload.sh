#/bin/sh

files="wp34s/revision.txt wp34s_V3.zip"
docs="wp34s/doc/*.pdf wp34s/doc/flash.txt"
user="mvcube,wp34s"
dest="frs.sourceforge.net:/home/frs/project/w/wp/wp34s"

echo README.TXT needs to be updated from the web interface
scp $files $user@$dest
scp $docs $user@$dest/doc
