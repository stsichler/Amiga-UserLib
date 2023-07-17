if not exists include:user
makedir include:user
endif
copy user.h include:user/user.h
copy functions.h include:user/functions.h
copy pointerlists.h include:user/pointerlists.h
copy exceptions.h include:user/exceptions.h


cd global
sc:c/smake
cd /maths
sc:c/smake
echo "*N*N*E[1mClose Window by clicking the close gadget.*e[0m"
