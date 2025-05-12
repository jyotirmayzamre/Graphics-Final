all:
	g++ -Iinclude src/*.cpp -o raytracer; \
	./raytracer > sphere.ppm; \
	display sphere.ppm