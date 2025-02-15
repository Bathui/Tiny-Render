#include "tgaimage.h"
#include "model.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const int width = 2000;
const int height = 2000;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) { 
    bool steep = false;
	if (std::abs(x0-x1) < std::abs(y0-y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}

	if (x1 < x0) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy)*2;
	int error = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) { 
		if (steep){
			image.set(y, x, color);
		}
		else{
			image.set(x, y, color);
		}
		error += derror2;
		if (error> dx){
			y += (y1>y0? 1:-1);
			error -= 2*dx;
		}		
    } 
}

vec3f baryCentric(vec2i* vertices, vec2i p){
    vec3f coefficient = vec3f(vertices[2].x - vertices[0].x, vertices[1].x - vertices[0].x, vertices[0].x - p.x) ^ vec3f(vertices[2].y - vertices[0].y, vertices[1].y - vertices[0].y, vertices[0].y - p.y);
    
    if(std::abs(coefficient.z) < 1)
        return vec3f(-1, 1, 1); // it means the triangle is trivial
    
    return vec3f(1.f-(coefficient.x+coefficient.y)/coefficient.z, coefficient.y/coefficient.z, coefficient.x/coefficient.z); // return the barycentric coefficient
}

void triangle(vec2i* pts, TGAImage& img, TGAColor color){
    vec2i boxmin(img.get_width()-1, img.get_height()-1);
    vec2i boxmax(0, 0);

    for(int i = 0; i < 3; i++){
        //Outer max is to handle the invalid cases
        boxmin.x = std::max(0, std::min(pts[i].x, boxmin.x));
        boxmin.y = std::max(0, std::min(pts[i].y, boxmin.y)); 
        
        boxmax.x = std::min(std::max(pts[i].x, boxmax.x), img.get_width()-1);
        boxmax.y = std::min(std::max(pts[i].y, boxmax.y), img.get_height()-1);
        
    }
    
    vec2i inter_points;
    for(inter_points.x = boxmin.x; inter_points.x<= boxmax.x; inter_points.x++) {
        for(inter_points.y = boxmin.y; inter_points.y<= boxmax.y; inter_points.y++){
            vec3f coefficient = baryCentric(pts, inter_points);
            if(coefficient.x < 0 ||coefficient.y < 0|| coefficient.z < 0)
                continue; //there are some points not in our simplex, so we need to skip those points
            img.set(inter_points.x, inter_points.y, color);
        }
    }

}

int main(int argc, char** argv) {
	const char* name = nullptr;
	if (argc == 1){
		name = "output.tga";
	}
	else{
		name = argv[1];
	}
	TGAImage image(width, height, TGAImage::RGB);
	Model* model = new Model("african_head.obj");
	// Model* model = new Model("sample.obj");
	// std::cout<<model->num_faces()<<std::endl;
    std::cout<<model->num_verts()<<std::endl;

	// for (int i = 0; i < model->num_faces(); i++) { 
	// 	std::vector<int> face = model->face(i);
	// 	// std::cout<<(model->face(0))[6]<<std::endl;
	// 	for (int j = 0; j < 3; j++) {
	// 		vec3f v0 = model->vert(face[j*3]);
	// 		vec3f v1 = model->vert(face[(j+1)%3*3]);
	// 		int x0 = (v0.X() + 1.) * width/2.;
	// 		int x1 = (v1.X() + 1.) * width/2.;
	// 		int y0 = (v0.Y() + 1.) * height/2.;
	// 		int y1 = (v1.Y() + 1.) * height/2.;
	// 		line(x0, y0, x1, y1, image, white);
	// 	}
	// }
    
    // for(int i = 0; i < model->num_faces(); i++){
    //     std::vector<int> face = model->face(i);
    //     vec2i screen[3];

    //     for (int j = 0; j < 3; j++) {
    //         vec3f world = model->vert(face[j*3]);
    //         screen[j].x = (world.x + 1.) * width / 2.;
    //         screen[j].y = (world.y + 1.) * height / 2.;

    //     }   
        
    //     triangle(screen, image, TGAColor(rand()%256, rand()%256, rand()%256, 255));        

    // }
    vec3f light_direction(0, 0, -1);
    
    for (int i = 0; i < model->num_faces(); i++) {
        vec3f world[3];
        vec2i screen[3];
        std::vector<int> face = model->face(i);

        for(int j = 0; j < 3; j++) {
            world[j] = model->vert(face[3*j]);
            screen[j] = vec2i((world[j].x + 1.) * width / 2, (world[j].y + 1.) * height / 2);  
        }
        vec3f normal = (world[2] - world[0]) ^ (world[1] - world[0]);
        normal.normalized();
        float intensity = normal * light_direction;

        if(intensity > 0)
            triangle(screen, image, TGAColor(255*intensity, 255*intensity, 255*intensity, 255));
        
    }
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file(name);
	delete model;
}

