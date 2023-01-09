AUTHOR = Bosch Pérez Valeria
PROJECT_NAME = P2-SSOO
DIR_PATH=$(shell realpath .)
DIR_NAME=$(shell basename ${DIR_PATH})

cmake:
	cmake -S . -B build

build: cmake
	cd build; make

clean:
	rm -rf build

tar:
	cd ..; tar cvfz ./${DIR_NAME}/${AUTHOR}-${PROJECT_NAME}.tar.gz --exclude-from=${DIR_NAME}/.gitignore ${DIR_NAME}
