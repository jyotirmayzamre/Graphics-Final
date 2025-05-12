all:
	g++ -Iinclude src/*.cpp -o raytracer; \
	./raytracer > image.ppm; \
	display image.ppm