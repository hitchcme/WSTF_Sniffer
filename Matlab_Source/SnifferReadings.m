function DATA = SnifferReadings()

	s = serial('/dev/cu.usbmodem1411')
	fopen(s);
	tline = fscanf(s);
	while tline
		disp(tline);
		tline = fscanf(s);
		
	end
	
