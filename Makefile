.PHONY: clean archive
SHELL=/bin/bash

clean:
	if [ -d "./release" ]; then \
	cd release; \
	if [ -f I2CSlaveMode.zip ]; then \
	rm I2CSlaveMode.zip; fi; \
	if [ -f I2CSlaveModeTrace.zip ]; then \
	rm I2CSlaveMode.zip; fi; \
	fi

archive: clean
	cd src && zip -r I2CSlaveMode.zip  I2CSlaveMode; 

trace-archive: clean
	cd src && zip -r I2CSlaveModeTrace.zip  I2CSlaveModeTrace; 

release: archive
	if [ ! -d "./release" ]; then mkdir release; fi; \
	mv src/I2CSlaveMode.zip release/.

trace-release: archive
	if [ ! -d "./release" ]; then mkdir release; fi; \
	mv src/I2CSlaveModeTrace.zip release/.

install: 
	cp -r src/I2CSlaveMode ~/Documents/Arduino/libraries/

install-trace: trace-archive 
	cp -r src/I2CSlaveModeTrace ~/Documents/Arduino/libraries/