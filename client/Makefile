GREEN='\033[0;32m'
NC='\033[0m' # No Color

all:
	python setup.py build_ext --inplace --force
	@echo -e ${GREEN}Build finished, running test script${NC}
	python test.py
	@echo -e ${GREEN}Script done!${NC}
