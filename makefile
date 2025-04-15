all:
	g++ -Iinclude src/*.cpp -o raytracer; \
	./raytracer > image.ppm; \
	feh image.ppm