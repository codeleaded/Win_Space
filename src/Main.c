#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Container/Vector.h"
#include "/home/codeleaded/System/Static/Container/Pair.h"
#include "/home/codeleaded/System/Static/Container/SQuadTree.h"
#include "/home/codeleaded/System/Static/Library/TransformedView.h"

typedef struct Ship {
    Vec2 p;
    Vec2 v;
    Vec2 ac;
	F32 r;
    F32 a;
} Ship;

Ship Me;
TransformedView tv;
Vector Objects;
float Area = 100000.0f;

void Ship_Update(Ship* s,float w->ElapsedTime){
	s->v = Vec2_Add(s->v,Vec2_Mulf(s->ac,w->ElapsedTime));
	if(Vec2_Mag(s->v)>10000.0f) s->v = Vec2_Mulf(Vec2_Norm(s->v),10000.0f);
	s->p = Vec2_Add(s->p,Vec2_Mulf(s->v,w->ElapsedTime));
}
void Ship_Render(Ship* s){
	Vec2 p1 = {         0.0f,-s->r * 0.66f };
    Vec2 p2 = { -s->r * 0.4f, s->r * 0.33f };
    Vec2 p3 = {  s->r * 0.4f, s->r * 0.33f };

    M2x2 rot = M2x2_RotateZ(s->a);
    p1 = M2x2_VecMul(p1,rot);
    p2 = M2x2_VecMul(p2,rot);
    p3 = M2x2_VecMul(p3,rot);

    p1 = Vec2_Add(p1,s->p);
    p2 = Vec2_Add(p2,s->p);
    p3 = Vec2_Add(p3,s->p);

	p1 = TransformedView_WorldScreenPos(&tv,p1);
	p2 = TransformedView_WorldScreenPos(&tv,p2);
	p3 = TransformedView_WorldScreenPos(&tv,p3);

    RenderLine(p1,p2,WHITE,1.0f);
    RenderLine(p2,p3,WHITE,1.0f);
    RenderLine(p3,p1,WHITE,1.0f);

	if(Vec2_Mag(s->ac)>0.0f){
		Vec2 p1 = {         0.0f, s->r * (float)Random_f64_MinMax(0.4,0.9) };
    	Vec2 p2 = { -s->r * 0.3f, s->r * 0.4f };
    	Vec2 p3 = {  s->r * 0.3f, s->r * 0.4f };
	
    	M2x2 rot = M2x2_RotateZ(s->a);
    	p1 = M2x2_VecMul(p1,rot);
    	p2 = M2x2_VecMul(p2,rot);
    	p3 = M2x2_VecMul(p3,rot);
	
    	p1 = Vec2_Add(p1,s->p);
    	p2 = Vec2_Add(p2,s->p);
    	p3 = Vec2_Add(p3,s->p);
	
		p1 = TransformedView_WorldScreenPos(&tv,p1);
		p2 = TransformedView_WorldScreenPos(&tv,p2);
		p3 = TransformedView_WorldScreenPos(&tv,p3);
		
		RenderLine(p2,p3,ORANGE,1.0f);
    	RenderLine(p3,p1,ORANGE,1.0f);
		RenderLine(p1,p2,ORANGE,1.0f);
	}
}

typedef struct Planet {
	Vec2 p;
	F32 r;
	Pixel c;
	Vector SubPlanets;
} Planet;

Planet Planet_New(Vec2 p,F32 r,Pixel c,int SubCount){
	Planet planet;
	planet.p = p;
	planet.r = r;
	planet.c = c;
	planet.SubPlanets = Vector_New(sizeof(Planet));
	
	for(int i = 0;i<SubCount;i++){
		Planet Obj = Planet_New(
			Vec2_Add(planet.p,Vec2_Mulf(Vec2_OfAngle(Random_f64_MinMax(0.0f,2*F32_PI)),planet.r + Random_f64_MinMax(500.0f,800.0f))),
			Random_f64_MinMax(2.0f,50.0f),
			Pixel_toRGBA(Random_f64_New(),Random_f64_New(),Random_f64_New(),1.0f),
			0
		);
		Vector_Push(&planet.SubPlanets,(Planet[]){ Obj });
	}
	return planet;
}
void Planet_Update(Planet* parent,Planet* obj,Rect Screen,size_t* ObjectCount){
	Vec2 d = Vec2_Sub(obj->p,parent->p);
	M2x2 r = M2x2_RotateZ(0.01f / obj->r);
	obj->p = Vec2_Add(parent->p,M2x2_VecMul(d,r));
}
void Planet_Render(Planet* obj,Rect Screen,size_t* ObjectCount){
	Vec2 pos = Vec2_Subf(obj->p,obj->r);
	Vec2 size = (Vec2){ obj->r*2.0f,obj->r*2.0f };
	if(Overlap_Rect_Rect(Screen,(Rect){ pos,size })){
		Vec2 p = TransformedView_WorldScreenPos(&tv,pos);
		Vec2 d = TransformedView_WorldScreenLength(&tv,size);
		//RenderRect(p.x,p.y,d.x,d.y,obj->c);
		RenderCircle(Vec2_Add(p,Vec2_Mulf(d,0.5f)),d.x*0.5f,obj->c);
		(*ObjectCount)++;
	}

	for(int i = 0;i<obj->SubPlanets.size;i++){
		Planet* sp = (Planet*)Vector_Get(&obj->SubPlanets,i);
		Planet_Update(obj,sp,Screen,ObjectCount);
		Planet_Render(sp,Screen,ObjectCount);
	}
}
void Planet_Free(Planet* p){
	for(int i = 0;i<p->SubPlanets.size;i++){
		Planet* sp = (Planet*)Vector_Get(&p->SubPlanets,i);
		Planet_Free(sp);
	}
	Vector_Free(&p->SubPlanets);
}


void Setup(AlxWindow* w){
	Me = (Ship){ Area * 0.5f,Area * 0.5f,0.0f,0.0f,0.0f,0.0f,10.0f,0.0f };
	tv = TransformedView_New((Vec2){ GetWidth(),GetHeight() });
	TransformedView_Focus(&tv,&Me.p);

    Objects = Vector_New(sizeof(Planet));

	for(int i = 0;i<100;i++){
		Planet Obj = Planet_New(
			(Vec2){ Random_f64_MinMax(0.0f,Area),Random_f64_MinMax(0.0f,Area) },
			Random_f64_MinMax(100.0f,1000.0f),
			Pixel_toRGBA(Random_f64_New(),Random_f64_New(),Random_f64_New(),1.0f),
			Random_i32_MinMax(0,5)
		);
		Vector_Push(&Objects,(Planet[]){ Obj });
	}
}
void Update(AlxWindow* w){
    if(Stroke(ALX_KEY_W).DOWN){
        M2x2 rot = M2x2_RotateZ(Me.a);
        Vec2 Dir = { 0.0f,-1.0f };
        Dir = M2x2_VecMul(Dir,rot);
        Dir = Vec2_Mulf(Dir,100.0f);
        Me.ac = Dir;
    }else if(Stroke(ALX_KEY_S).DOWN){
        M2x2 rot = M2x2_RotateZ(Me.a);
        Vec2 Dir = { 0.0f,1.0f };
        Dir = M2x2_VecMul(Dir,rot);
        Dir = Vec2_Mulf(Dir,100.0f);
        Me.ac = Dir;
    }else{
		Me.ac = (Vec2){ 0.0f,0.0f };
	}
    if(Stroke(ALX_KEY_A).DOWN){
        Me.a -= 1.5f * 3.14f * w->ElapsedTime;
    }
    if(Stroke(ALX_KEY_D).DOWN){
        Me.a += 1.5f * 3.14f * w->ElapsedTime;
    }

	Ship_Update(&Me,w->ElapsedTime);

	TransformedView_HandlePanZoom(&tv,window.Strokes,(Vec2){ GetMouse().x,GetMouse().y });
	Rect Screen = TransformedView_Rect(&tv,(Rect){ 0.0f,0.0f,GetWidth(),GetHeight() });
	size_t ObjectCount = 0;
	
	Clear(BLACK);
	
	Timepoint start = Time_Nano();
	for(int i = 0;i<Objects.size;i++){
		Planet* obj = (Planet*)Vector_Get(&Objects,i);
		Planet_Render(obj,Screen,&ObjectCount);
	}
	Ship_Render(&Me);
	FDuration ElapsedTime = Time_Elapsed(start,Time_Nano());

	String FStr = String_Format("Linear / %d in %f",ObjectCount,ElapsedTime);
	//printf("Linear / %d in %f vs %s\n",ObjectCount,ElapsedTime,cstr);
	RenderCStrSize(FStr.Memory,FStr.size,0.0f,0.0f,RED);
	String_Free(&FStr);
}
void Delete(AlxWindow* w){
	for(int i = 0;i<Objects.size;i++){
		Planet* obj = (Planet*)Vector_Get(&Objects,i);
		Planet_Free(obj);
	}
    Vector_Free(&Objects);
}

int main(){
    if(Create("Quad Trees",2500,1300,1,1,Setup,Update,Delete))
        Start();
    return 0;
}