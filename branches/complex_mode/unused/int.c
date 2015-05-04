long long int intFactorial(long long int x) { 	 
	int s; 	 
	unsigned long long int xv = extract_value(x, &s); 	 
	unsigned int n, i; 	 

	if (xv > 65) { 	 
		set_overflow(1); 	 
		return 0; 	 
	} 	 

	n = xv & 0xff; 	 
	xv = 1; 	 

	for (i=n; i>1; i--) 	 
		xv *= i; 	 
	set_overflow(n > 20 || check_overflow(xv)); 	 
	return build_value(xv, 0); 	 
}


/* Integer cube root
 */
long long int intCubeRoot(long long int v) {
	int sx;
	unsigned long long int w = extract_value(v, &sx);
	unsigned long long int x, y, b, bs, y2;
	int s;

	if (w == 0) {
		set_carry(0);
		return 0;
	}
	x = w;
	y2 = 0;
	y = 0;
	for (s=63; s>=0; s -= 3) {
		y2 <<= 2;
		y <<= 1;
		b = 3*(y2 + y) + 1;
		bs = b << s;
		if (x >= bs && b == (bs >> s)) {
			x -= bs;
			y2 += (y << 1) + 1;
			y++;
		}
	}
	set_carry((y*y*y != w)?1:0);
	return build_value(y, sx);
}



/* Integer xth root piggy backs on the real code. 	 
 */ 	 
long long int intXRoot(long long int y, long long int x) {
	int sx, sy;
	unsigned long long int vx = extract_value(x, &sx);
	unsigned long long int vy = extract_value(y, &sy);
	decNumber rx, ry, rz;

	if (sx) {
		if (vy == 1) {
			set_carry(0);
			vy = 1;
		} else {
			set_carry(1);
			vy = 0;
		}
		if (sy != 0)
			sy = vx & 1;
		return build_value(vy, sy);
	}

	ullint_to_dn(&rx, vx);
	ullint_to_dn(&ry, vy);
	decNumberXRoot(&rz, &ry, &rx);
	if (decNumberIsSpecial(&rz)) {
		set_overflow(1);
		set_carry(0);
		return 0;
	}
	vy = dn_to_ull(decNumberFloor(&rx, &rz), &sx);
	if (sy) {
		sx = (vx & 1) ? 0 : 1;
		y = -y;
	} else
		sx = 0;
	set_carry(intPower(vy, x) != y);
	set_overflow(sx);
	return build_value(vy, sy);
}

