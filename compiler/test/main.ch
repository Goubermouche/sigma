// todo: explicit cast (!)
//  - cast<i32>(value)

// todo: references (!!)
//  - add support for references (&)

// todo: general
//  - break (!!!)
//  - continue (!!)
//  - delete[] (!!)
//  - delete (!!)
//  - new for single elements (!!)

// todo: major cleanup (!!!)


i32 main() {
	for(i32 i = 1; i <= 100; ++i) {
		if (i % 15 == 0) {
			print("FizzBuzz\t");   
		}
		else if(i % 3 == 0) {
			print("Fizz\t");
		}
		else if(i % 5 == 0) {
            print("Buzz\t");  
		}      
		else {
			print("%d\t", i);    
		}
	}

	return 0;
}


// https://github.com/rkosegi/mandelbrot_console
//u64 ROWS = 20;
//u64 COLS = 80;
//i32* array;
//
//void set_pixel(i32* array, i32 cols, i32 rows, i32 x, i32 y, i32 value) {
//	i32 idx = x + cols * y;
//	array[idx] = value;
//}
//
//void mandelbrot(i32* array, i32 cols, i32 rows) {
//	f64 min_re = -2.0;
//	f64 max_re = 2.0;
//	f64 min_im = -1.2;
//	f64 max_im = min_im + (max_re - min_re) * rows / cols;
//	f64 re_factor = (max_re - min_re) / (cols - 1);
//	f64 im_factor = (max_im - min_im) / (rows - 1);
//	u64 max_iterations = 30;
//
//	for(u64 y = 0; y < rows; y++) {
//		f64 c_im = max_im + y * im_factor;
//
//		for(u64 x = 0; x < cols; x++) {
//			f64 c_re = min_re + x * re_factor;
//
//			f64 z_re = c_re;
//			f64 z_im = c_im;
//			bool is_inside = true;
//
//			for(u64 n = 0; n < max_iterations; n++) {
//				f64 z_re2 = z_re * z_re;
//				f64 z_im2 = z_im * z_im;
//
//				if(z_re2 + z_im2 > 4) {
//					is_inside = false;
//					break;
//				}
//
//				z_im = 2 * z_re * z_im + c_im;
//				z_re = z_re2 - z_im2 + c_re;
//			}
//
//			if(is_inside) {
//				set_pixel(array, cols, rows, x, y, 1);
//			}
//		}
//	}
//}
//
//void dump(i32* array, i32 cols, i32 rows) {
//	for(i32 idx = 0; idx < rows * cols; idx++) {
//		print("%u", array[idx]);
//
//		if(((idx + 1) % cols) == 0) {
//			print("\n");
//		}
//	}
//}
//
//i32 main() {
//	u64 array_size = ROWS * COLS;
//	array = new i32[array_size];	
//
//	// memset 0
//	for(u64 i = 0; i < array_size; i++) {
//		array[i] = 0;
//	}
//
//	mandelbrot(array, COLS, ROWS);
//	dump(array, COLS, ROWS);
//
//	return 0;
//}
//