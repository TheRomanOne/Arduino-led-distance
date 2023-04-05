int cols[8] = {8, 3, 13, A3, 9, 11, 5, 4};
int rows[8] = {2, 7, 10, 6, 12, A5, A4, A2};

#define trigPin A1
#define echoPin A0
#define button 1

#define MAX_DISTANCE_CM 25
#define MIN_DISTANCE_CM 3

#define __DEBUG__ false
#define __SNAKE_TIMEOUT__ 350
#define snake_length 1



class vec2
{
  private:
    void calculate_length(){length = sqrt(pow(x, 2) + pow(y, 2));};
  public:
    float x, y, length;
    vec2() {x=0;y=0;};

    vec2(float _x, float _y)
    {
      x=_x;y=_y;
      calculate_length();
    };

    vec2 add(vec2 v)
    {
        return vec2(x+v.x, y+v.y);
    };

    vec2 mul(float v)
    {
      return vec2(x*v, y*v);
    }

    vec2 normalize()
    {
      return vec2(x/length, y/length);
    };
    
};

void trigger_pixel(vec2 p, float t)
{
  turn_pixel_on(p.x, p.y);
  delay(t);
  turn_pixel_off(p.x, p.y);
}
int sign(float x) { return abs(x)/x; }

float normalize_distance(float d)
{
  // Normalize distance between 0 and 1
  float mn = MIN_DISTANCE_CM;
  float mx = MAX_DISTANCE_CM;
  float l = mx - mn;
  float da = min(d, mx);
  da = max(da, mn) - mn;
  return da / l;
}

float get_speed(float d)
{
  return __SNAKE_TIMEOUT__ * normalize_distance(d);
}

class mat2
{
  private:
  public:
    float values[4] = {0, 0, 0, 0};
    mat2(float _v0, float _v1, float _v2, float _v3)
    {
      values[0]=_v0;values[1]=_v1;
      values[2]=_v2;values[3]=_v3;
    };

    vec2 mul(vec2 v)
    {
      return vec2(
        v.x * values[0] + v.y * values[1],
        v.x * values[2] + v.y * values[3]
      );
    }
};

class Square
{
  private:
    int __CURRENT_POINT__ = 0;
    int __ARRAY_SIZE__ = 12;
    void _recalculate()
    {
      vec2 l1 = points[3].add(points[0].mul(-1));
      vec2 l2 = points[6].add(points[3].mul(-1));
      vec2 l3 = points[9].add(points[6].mul(-1));
      vec2 l4 = points[0].add(points[9].mul(-1));
      points[1]= points[0].add(l1.mul(.33));
      points[2]= points[0].add(l1.mul(.66));
      points[4]= points[3].add(l2.mul(.33));
      points[5]= points[3].add(l2.mul(.66));
      points[7]= points[6].add(l3.mul(.33));
      points[8]= points[6].add(l3.mul(.66));
      points[10]=points[9].add(l4.mul(.33));
      points[11]=points[9].add(l4.mul(.66));
    }
  public:
    vec2 position;
    float scale;
    vec2 points[12];
    float draw_speed=1;
    Square()
    {
      points[0] = vec2(1, -1);
      points[3] = vec2(1, 1);
      points[6] = vec2(-1, 1);
      points[9] = vec2(-1, -1);
      _recalculate();
    };

    Square(vec2 v0, vec2 v1, vec2 v2, vec2 v3)
    {
      points[0] = v0;
      points[3] = v1;
      points[6] = v2;
      points[9] = v3;
      _recalculate();
      
    };

    void set_position(vec2 p)
    {
      for(int i=0; i < __ARRAY_SIZE__; i++)
        points[i] = points[i].add(p.add(position.mul(-1)));        
      position = p;
    }
    
    void set_scale(float s)
    {
      scale = s;
      for(int i=0; i < __ARRAY_SIZE__; i++)
      {
        vec2 _p = position.mul(-1);
        vec2 v = points[i].add(_p);
        v = v.mul(s/v.length);
        points[i] = v.add(position);        
      }
    }
    
    void rotate(float a)
    {
      float s = sin(a);
      float c = cos(a);
      mat2 m = mat2(c, -s, s, c);
      vec2 _p = position.mul(-1);
      for(int i=0; i < 8; i++)
      {
        vec2 v = points[i].add(_p);
        v = m.mul(v);
        points[i] = v.add(position);        
      }
    };
    
    void draw()
    {
      trigger_pixel(points[__CURRENT_POINT__], draw_speed);
      __CURRENT_POINT__ = (__CURRENT_POINT__ + 1) % __ARRAY_SIZE__; 
    }

    int get_array_size()
    {
      return __ARRAY_SIZE__;
    };
};

int __INIT__ = 0;
int __GAME_NUM__ = 0;
vec2 origin[snake_length]={};
vec2 direction;
Square thesquare = Square();

int square_counter = 0;
// int __SQUARE_POINTS__ 4;

float get_distance()
{
  // Read sonal signal and calculate distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float duration = pulseIn(echoPin, HIGH);
  return (duration / 2) * 0.0343;
}



void turn_pixel_on(int x, int y)
{
  if(x < 0 || x > 7 || y < 0 || y > 7)
  return;
  digitalWrite(rows[y], HIGH);
  digitalWrite(cols[x], LOW);
}

void turn_pixel_off(int x, int y)
{
  if(x < 0 || x > 7 || y < 0 || y > 7)
  return;
  digitalWrite(rows[y], LOW);
  digitalWrite(cols[x], HIGH);
}



void blink(vec2 v, int t)
{
  trigger_pixel(v, t);
  delay(t*.2);
}

void test_screen()
{
  // Runs line animation to test LEDs
  for(int side = 0; side < 4; side++)
  {
    for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
    {
      if(side == 0) blink(vec2(i, j), 7);
      else if(side == 2) blink(vec2(7-i, j), 7);
      else if(side == 1) blink(vec2(j, i), 7);
      else blink(vec2(j, 7-i), 7);
    }
  }
}



vec2 get_uc(float x, float y)
{
  // Center origin in the middle of the screen
   return vec2(x - 3.5, y - 3.5);
}






// Visualizations

void print_vector(vec2 v)
{
  Serial.print(v.x);
  Serial.print(", ");
  Serial.println(v.y);
}

void level()
{
  float distance = get_distance();
  float p = normalize_distance(distance);
  float fp = floor(p*7);
  for(int q = 0; q < 8; q++)
    blink(vec2(q, fp), 5);
}

void ball()
{
  float distance = get_distance();
  float p = normalize_distance(distance);
  for(int i = 0; i < 8; i++)
  for(int j = 0; j < 8; j++)
  {
    vec2 v = get_uc(i, j);
    if(v.length < p*4)      
      trigger_pixel(vec2(i, j), 1);
    }
}

vec2 rotate(vec2 v, float a)
{
  // return vec2(v.x * cos(a) - v.y * sin(a), v.x * sin(a) + v.y * cos(a));
  mat2 m = mat2(cos(a), -sin(a), sin(a), cos(a));
  return m.mul(v);
}
void square()
{
  Square e = thesquare;
  
  vec2 p = vec2(0, 4);
  e.set_position(p);
  float a = 0.1;    
  float cycle = 45;
  for(int i=0; i < cycle; i++)
  {
    float distance = get_distance();
    float speed = normalize_distance(distance);

    if(speed > .49 && speed < .51)
      square_counter++;
    
    e.set_scale(2 + (speed-.5) * 7);
    e.set_position(p.add(vec2(8*speed, (speed-.5)*3*sin(2. * i * 3.1415/cycle) ) ) );
    e.rotate((speed-.5)*3.1415/180);
    if(square_counter == 100)  
    {
      delay(750);
      e.draw_speed = 100;
      square_counter = 0;
      e.set_scale(3);
      for(int w=0; w < 2; w++)
      {
        for(int q=0; q < e.get_array_size(); q++)
          e.draw();
        e.set_scale(e.scale-1);
        delay(750);
      }
      delay(750);
      e.draw_speed = 1;
    }else e.draw();
  }
  
}

void dot()
{
    float distance = get_distance();
    float speed = get_speed(distance);
    if(__INIT__ == 0)
    {
      // Generate random starting point
      vec2 o = vec2(7*random(8.)/7., 7*random(8.)/7.);
      origin[0] = o;
      // Generate random starting direction between [-1, -1] and [1, 1]
      vec2 dir = vec2(-1. + random(200)/200., -1. + random(200)/200.);
      direction = dir;

      for(int l=0; l < snake_length-1; l++)
        origin[l+1] = origin[l].add(dir);

      __INIT__ = 1;
    }else
    {
      vec2 o = origin[0];
      vec2 d = direction;

      for(int l=0; l < snake_length; l++)
        trigger_pixel(origin[l], 5 + speed);
        // trigger_pixel(origin[l], l==0?5 + __SNAKE_TIMEOUT__*p:5);
      
      vec2 no = o.add(d);
      if(no.x <= 0 || no.x >= 7) 
      {
        // switch direction from wall
        no.x = min(7, max(0, no.x));
        d.x *= -1;
        d.y += (random(100)/100.) * sign(d.x);
        d = d.normalize();
      }
      if(no.y <= 0 || no.y >= 7) 
      {
        // switch direction from wall
        no.y = min(7, max(0, no.y));
        d.y *= -1;
        d.x += (random(100)/100.) * sign(d.y);
        d = d.normalize();
      }
      direction = d;
      for(int l=snake_length; l > 1; l--)
          origin[l-1]=origin[l-2];

      origin[0] = no;
    }
}

void setup()
{
  if(__DEBUG__) Serial.begin (9600);
  pinMode(button, INPUT);
  for(int i = 0; i < 8; i ++)
  {
    pinMode(rows[i], OUTPUT);
    pinMode(cols[i], OUTPUT);
    digitalWrite(rows[i], LOW);
    digitalWrite(cols[i], HIGH);
  }
  if(!__DEBUG__)test_screen();
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop()
{
  int click = digitalRead(button);
  if(!__DEBUG__ && click==1)
  {
    __GAME_NUM__ = (__GAME_NUM__ + 1) % 4;
    delay(750);
  }
  switch(__GAME_NUM__)
  {
    case 0: level(); break;
    case 1: dot(); break;
    case 2: ball(); break;
    case 3: square(); break;
  }
}