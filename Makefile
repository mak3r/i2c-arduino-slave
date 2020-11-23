.PHONY: clean archive
SHELL=/bin/bash

clean:
	if [ -d "./release" ]; then \
	cd release; \
	if [ -f I2CSlaveMode.zip ]; then \
	rm I2CSlaveMode.zip; fi; \
	fi

archive: clean
	cd src && zip -r I2CSlaveMode.zip  I2CSlaveMode; 

release: archive
	if [ ! -d "./release" ]; then mkdir release; fi; \
	mv src/I2CSlaveMode.zip release/.