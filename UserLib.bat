if not exists include:user
makedir include:user
makelink include:user/user.h user.h
makelink include:user/functions.h functions.h
makelink include:user/pointerlists.h pointerlists.h
makelink include:user/exceptions.h exceptions.h
endif

cd global
sc:c/smake
cd /maths
sc:c/smake
echo "*N*N*E[1mClose Window by clicking the close gadget.*e[0m"
