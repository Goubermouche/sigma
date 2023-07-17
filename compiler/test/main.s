
void donut(f32 rot_speed) {
	f32 A = 0;
	f32 B = 0;
	f32* z = new f32[1760];
	char* b = new char[1760];
	char* brightness = ".,-~:;=!*#$@";
	print("\x1b[2J");
	
	while(true) {
		memset(b, 32, 1760);
		memset(z, 0, 7040);

		for(f32 j = 0; j < 6.28; j = j + 0.07f) {
			for(f32 i = 0; i < 6.28; i = i + 0.02f) {
				f32 c = sin(i);
				f32 d = cos(j);
				f32 e = sin(A);
				f32 f = sin(j);
				f32 g = cos(A);
				f32 h = d + 2;
				f32 D = 1 / (c * h * e + f * g + 5);
				f32 l = cos(i);
				f32 m = cos(B);
				f32 n = sin(B);
				f32 t = c * h * g - f * e;
				i32 x = 40 + 30 * D * (l * h * m - t * n);
				i32 y = 12 + 15 * D * (l * h * n + t * m);
				i32 o = x + 80 * y;
				i32 N = 8 * ((f * e - c * d * g) * m - c * d * e - f * g - l * d * n);
		
				if(1760 > o && o > 0 && D > z[o]) {
					z[o] = D;
		
					if(N > 0) {
						b[o] = brightness[N];
					}
					else {
						b[o] = brightness[0];
					}
				}
			}
		}
		
		print("\x1b[H");
		
		for(i32 k = 0; k < 1760; k++) {
			if(k % 80 != 0) {
				printc(b[k]);
			}
			else {
				printc('\n');
			}
		}
		
		A = A + rot_speed;
		B = B + rot_speed;
	}

	free(z);
	free(b);
}

i32 main() {
	donut(0.1f);
	return 0;
}