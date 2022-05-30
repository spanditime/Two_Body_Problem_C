#include <stdio.h>
#include <math.h>
#include <png.h>
#include <math.h>
#include <stdlib.h>

#define EPS 10


typedef struct {
    float x, y;
} vec2f; // структура вектор (x,y)

typedef struct{
    vec2f position;
    vec2f speed;
    vec2f acceleration;
    float mass;
    float radius;
    vec2f*trail;
    //color - цвет используемый для отрисовки
} body; // тело, хранит данные о теле и его пути

vec2f sum_vec(vec2f f, vec2f s){ // сложение
    f.x += s.x;
    f.y += s.y;
    return f;
} // возвращает вектор явл суммой переданных векторов

vec2f sub_vec(vec2f f, vec2f s){ // вычит векторов
    f.x -= s.x;
    f.y -= s.y;
    return f;
}

vec2f mul_vec(vec2f f, float s){ // умножения вектора на число
    f.x *= s;
    f.y *= s;
    return f;
}

vec2f div_vec(vec2f f, float s){ // деление вектора на число
    f.x *= s;
    f.y *= s;
    return f;
}

float len_vec(vec2f v){ // длина вектора
    return sqrt(v.x*v.x + v.y*v.y);
}

vec2f normalize(vec2f v){ // нормализация вектора, что бы у вектора была длина 1
    float l = len_vec(v);
    v.x /= l;
    v.y /= l;
    return v;
}

int width = 1000, height = 1000; // размер выходного изображения(должен быть квадратным, тк лень было делать что бы подгонялся размер);
unsigned char *buff;


// отрисовка круга в буффер
void drawCircle(float x_c, float y_c, float R, unsigned char r, unsigned char g, unsigned char b){
    vec2f p; p.x = x_c; p.y = y_c; // p - точка центра, R - радиус, r g b - цвет 
    for(int x = x_c-R-2; x < x_c+R+2; x++){
        for(int y = y_c-R-2; y < y_c+R+2; y++){ // рассматриваем все пиксели в буффере в диапазоне от (p.x-R-2, p.y-R-2) до (p.x+R+2, p.x+R+2)
        // по идее вместо +2 можно брать +1, но так как у нас  коорд центра вещественные числа а они округляются вниз взято -2
            if(x<0||x>=width||y<0||y>=height) // если выходит за изображение, хотя учитывая входные данные такого не может быть, но на всякий случай
                continue; // пропускаем текущую итерацию
            vec2f t; t.x = x; t.y = y; // текущая обрабатываемая точка
            float length = len_vec(sub_vec(p,t));
            // printf("x_c=%f, y_c=%f, r=%f, l=%f, x,y=%d,%d\n",x_c,y_c,R,len_vecgth,x,y);
            if(length<=R){
                buff[y*width*3+x*3  ] = r;
                buff[y*width*3+x*3+1] = g;
                buff[y*width*3+x*3+2] = b;
            }else if(length<=R+1){
                float op = R+1-length;
                buff[y*width*3+x*3  ] = op*r + (1-op)*buff[y*width*3+x*3  ];
                buff[y*width*3+x*3+1] = op*g + (1-op)*buff[y*width*3+x*3+1];
                buff[y*width*3+x*3+2] = op*b + (1-op)*buff[y*width*3+x*3+2];
            }

        }
    }
}


void step(body *b, float timespan){
    b->speed = sum_vec(b->speed, mul_vec(b->acceleration,timespan)); //speed += acceleration * deltaT 
    b->position = sum_vec(b->position, mul_vec(b->speed,timespan)); // position += speed * deltaT
}

void resolve(body* f, body*s, float timespan){
    vec2f dir = sub_vec(f->position,s->position); // вектор направления от первого тела ко второму
    float distance = len_vec(dir);
    float force = 0;
    if(distance>=EPS){ // во избежание деления на числа близкие нулю
        force = 66.7f * f->mass * s->mass / (distance*distance);
        dir = normalize(dir);
        f->acceleration = mul_vec(dir, -force/s->mass);
        s->acceleration = mul_vec(dir, force/f->mass);
    }else{
        f->acceleration = (vec2f){0,0};
        s->acceleration = (vec2f){0,0};
    }
    step(f,timespan);
    step(s,timespan);
}

// параметры передаваемые программе, отмеченные * не обязательные
// input: {name of exe} {f.mass} {f.pos.x} {f.pos.y} {f.speed.x} {f.speed.y} {f.radius} {s.mass} {s.pos.x} {s.pos.y} {s.speed.x} {s.speed.y} {s.radius} {*filename}
int main(int argc, char**argv){
    // переменные необходимые для создания и сохранения пнг
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;
    char*filename = "./result.png";

    // выделяем место под буффер размером ширина*длинна*3 тк у нас 3 цвета r g b
    buff = (unsigned char*)malloc(width*height*3*sizeof(unsigned char));
    for(int i = 0; i < width*height*3; i++ ){
        buff[i] = 0; // заполняем нулями, черный фон что бы был
    }

    body f, s;
    int seconds;
    if(argc == 14||argc == 15){
        sscanf(argv[1],"%f",&(f.mass)); // записываем из аргументов переданных программе данные наших тел
        sscanf(argv[2],"%f",&(f.position.x));
        sscanf(argv[3],"%f",&(f.position.y));
        sscanf(argv[4],"%f",&(f.speed.x));
        sscanf(argv[5],"%f",&(f.speed.y));
        sscanf(argv[6],"%f",&(f.radius));
        sscanf(argv[7],"%f",&(s.mass));
        sscanf(argv[8],"%f",&(s.position.x));
        sscanf(argv[9],"%f",&(s.position.y));
        sscanf(argv[10],"%f",&(s.speed.x));
        sscanf(argv[11],"%f",&(s.speed.y));
        sscanf(argv[12],"%f",&(s.radius));
        sscanf(argv[13],"%d",&(seconds));
        if(argc == 15){
            filename = argv[14];
        }
    }else{
        // если не передали аргументы устанавливаем по умолчанию
        f.position = (vec2f){-100,0};
        f.speed = (vec2f){0,-3};
        f.mass = 100;
        f.radius = 10;
        s.position = (vec2f){100,0};
        s.speed = (vec2f){0,3};
        s.mass = 100;
        s.radius = 10;
        seconds = 87;
    }

    // выделяем место под хранение истории перемещения тел, просто набор точек где было тело в опр шаг
    f.trail = (vec2f*)malloc(seconds*6*sizeof(vec2f));
    s.trail = (vec2f*)malloc(seconds*6*sizeof(vec2f));

    // Открываем файл для записи в бинарном режиме
    fp = fopen(filename, "wb");
    if (fp == NULL) { // если файл открыть не удалось выводим сообщение об ошибке и прерываем выполнение с кодом 1
        fprintf(stderr, "Could not open file %s for writing\n", filename);
        return 1;
    }

    // выводим на экран начальные данные тел
    printf("first body { mass=%f pos=(%f,%f) speed=(%f,%f) }\n",f.mass,f.position.x,f.position.y, f.speed.x, f.speed.y);
    printf("second body { mass=%f pos=(%f,%f) speed=(%f,%f) }\n",s.mass,s.position.x,s.position.y, s.speed.x, s.speed.y);
    if(seconds<=0||f.mass<=0||s.mass<=0){
        printf("mass or time cant be 0\n");
        return 1;
    }

    // Инициализируем структуру записи для пнг
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) { // сообщение об ошибке в случае неудачи
        fprintf(stderr, "Could not allocate write struct\n");
        return 1;
    }

    // Initialize info structure
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fprintf(stderr, "Could not allocate info struct\n");
        return 1;
        
    }

    // взято из интернета, обработка ошибок для pnglib, в случае ошибки будет выполнятся данный кусок кода, выводить сообщение об ошибке
    // Setup Exception handling 
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Error during png creation\n");
		return 1;
	}

    // указываем наш файл как поток вывода для png
    png_init_io(png_ptr, fp);

    // заголовок png файла, глубина цвета 8бит, rgb режим, все остальное по умолч
    png_set_IHDR(png_ptr, info_ptr, width, height,
            8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    
    // Set title
	// {
	// 	png_text title_text;
	// 	title_text.compression = PNG_TEXT_COMPRESSION_NONE;
	// 	title_text.key = "Title";
	// 	title_text.text = "result";
	// 	png_set_text(png_ptr, info_ptr, &title_text, 1);
	// }

    // записываем info (сам не особо в курсе зачем, но так надо)
	png_write_info(png_ptr, info_ptr);


    // сама симуляция
    for(int i = 0; i < seconds*60;i++){ // 60 шагов за секунду
        if(i%10 == 0){
            //сохраняем позицию тела каждые 10 шагов
            f.trail[i/10] = f.position;
            s.trail[i/10] = s.position;
        }
        resolve(&f, &s, 1./60.); // соответственно в качества шага берем 1/60 секунды
    }

    float roff = fmax(f.radius,s.radius);// устанавливаем отступ в виде наибольшего радиуса, что бы не выходили тела за края изображения

    // ищем минимальные и максимальные x и y, что бы высчитать zoom скажем так, что бы соответственно уменьшить все и переместить, что бы все влезло на картинку
    float xmax= f.position.x, xmin = f.position.x, ymax=f.position.y, ymin=f.position.y;
    xmax = fmax(xmax, s.position.x);
    xmin = fmin(xmin, s.position.x);
    ymax = fmax(ymax, s.position.y);
    ymin = fmin(ymin, s.position.y);
    for(int i = 0; i < seconds*6; i++){
        xmax = fmax(xmax, fmax(f.trail[i].x,s.trail[i].x));
        xmin = fmin(xmin, fmin(f.trail[i].x,s.trail[i].x));
        ymax = fmax(ymax, fmax(f.trail[i].y,s.trail[i].y));
        ymin = fmin(ymin, fmin(f.trail[i].y,s.trail[i].y));
    }
    ymin -= roff;
    xmin -= roff;
    xmax += roff;
    ymax += roff;// добавляем к ним еще и максимальный радиус, что бы в случае когда тело на краю оно не выходило за изображение
    float scale = 980./fmax((xmax-xmin),(ymax-ymin)); // изображение у нас 1000 на 1000, делим 980 на максимальную длину между объектами по x или y (на наибольшее из этих двух)
    // что бы вычислить коэфицент на который нужно умножить наши координаты смещенные на (-xmin,-ymin) что бы у нас все объекты не выходили за пределы от (0,0) до (980,980)

    // в итоге для того что бы вычислить координаты на изображении нам надо
    // imgcoord(x или y) = (coord(x или y) - (-xmin,-ymin))*scale +10
    // imgcoord - итоговые координаты точки на изображении,
    // cooord координаты точки в нашей симуляции которую мы хотим найти координаты на изображении
    // (-xmin,-ymin) - смещение что бы отрисовка было от нуля
    // +10 - что бы оставить по 10 пикселей с краев



    // рисуем маленькими точками с радиусом 1
    // из за этого отчетливо можно увидеть где тела ускорялись а где замедлялись по плотности точек, чем больше расстояние между точками тем быстрее двигалось тело, там где они сливаются в линию тело двигалось медленно
    for(int i = 0; i < seconds*6; i++){
        drawCircle((f.trail[i].x-xmin)*scale + 10, (f.trail[i].y-ymin)*scale + 10, 0.5, 193, 70, 63); // прибавляем ко всем координатам еще 10, что бы у нас был бордер в 10 пикселей по краям
        drawCircle((s.trail[i].x-xmin)*scale + 10, (s.trail[i].y-ymin)*scale + 10, 0.5, 63, 70, 193);
    }
    // отрисовываем сами тела
    drawCircle((f.position.x-xmin)*scale + 10, (f.position.y-ymin)*scale + 10, f.radius*scale, 255,0,0);
    drawCircle((s.position.x-xmin)*scale + 10, (s.position.y-ymin)*scale + 10, s.radius*scale, 0,0,255);



    // выделяем память под строку пикселей
    row = (png_bytep) malloc(3 * width * sizeof(png_byte));
    // построчно записываем в изображение
    for (int y=0 ; y<height ; y++) {
        for (int x=0 ; x<width ; x++) {
            row[x*3  ] = buff[y*width*3 + x*3  ];// red - в массиве последовательно записан каждый пиксель в формате r g b (тоже последовательно)
            row[x*3+1] = buff[y*width*3 + x*3+1];// green
            row[x*3+2] = buff[y*width*3 + x*3+2];// blue
        }
        png_write_row(png_ptr, row); // записываем эту строку
    }

    png_write_end(png_ptr, NULL); // заканчиваем запись


    // выводим итоговые значения свойств тел
    printf("first body { mass=%f pos=(%f,%f) speed=(%f,%f) }\n",f.mass,f.position.x,f.position.y, f.speed.x, f.speed.y);
    printf("second body { mass=%f pos=(%f,%f) speed=(%f,%f) }\n",s.mass,s.position.x,s.position.y, s.speed.x, s.speed.y);

    // освобождаем все динамические ресурсы
    if (fp != NULL) fclose(fp);
    if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    if (row != NULL) free(row);

    return 0;
}
