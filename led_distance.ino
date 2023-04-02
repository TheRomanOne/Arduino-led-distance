int cols[8] = {8, 3, 13, A3, 9, 11, 5, 4};
int rows[8] = {2, 7, 10, 6, 12, A5, A4, A2};

#define trigPin A1
#define echoPin A0

#define MAX_DISTANCE_CM 25
#define MIN_DISTANCE_CM 5

class vec2
{
  public:
    float x, y, length;
    vec2() {x=0;y=0;};

    vec2(float _x, float _y)
    {
      x=_x;y=_y;
      length = sqrt(pow(x, 2) + pow(y, 2));
    };

    vec2 add(vec2 v)
    {
        return vec2(x+v.x, y+v.y);
    };

    vec2 normalize()
    {
      return vec2(x/length, y/length);
    };
    
};


// Snake params
int __INIT__ = 0;
const int snake_length = 1;
vec2 origin[snake_length]={};
vec2 direction[1]={};

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

void setup()
{
  Serial.begin (9600);
  // delay(2000);
  for(int i = 0; i < 8; i ++)
  {
    pinMode(rows[i], OUTPUT);
    pinMode(cols[i], OUTPUT);
    digitalWrite(rows[i], LOW);
    digitalWrite(cols[i], HIGH);
  }
  test_screen();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
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

void trigger_pixel(vec2 p, float t)
{
  turn_pixel_on(p.x, p.y); delay(t);
  turn_pixel_off(p.x, p.y);
}

void blink(vec2 v, int t)
{
  trigger_pixel(v, t);
  delay(t*.2);
}

void test_screen()
{
  for(int side = 0; side < 4; side++)
  {
    for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
    {
      if(side == 0) blink(vec2(i, j), 7);
      else if(side == 1) blink(vec2(7-i, j), 7);
      else if(side == 2) blink(vec2(j, i), 7);
      else blink(vec2(j, 7-i), 7);
    }
  }
}

float normalize_distance(float d)
{
  float mn = MIN_DISTANCE_CM;
  float mx = MAX_DISTANCE_CM;
  float l = mx - mn;
  float da = min(d, mx);
  da = max(da, mn) - mn;
  return da / l;
}

vec2 get_uc(float x, float y)
{
   float nx = x - 3.5;
   float ny = y - 3.5;
   return vec2(nx, ny);
}

// Visualizations
void visualize(float distance)
{
  float p = normalize_distance(distance);
  int fp = floor(p*7);
  for(int q = 0; q < fp; q++)
  {
    int nx = q + 3 - int(fp/2);
    nx = max(nx, 1);
    blink(vec2(nx, fp), 5);
  }
}

void draw_ball(float distance)
{
    float p = normalize_distance(distance);
    for(int i = 0; i < 8; i++)
    for(int j = 0; j < 8; j++)
    {
      vec2 v = get_uc(i, j);
      if(v.length < p*4)      
        blink(vec2(i, j), 1);
    }
}

void snake(float distance)
{
    float p = normalize_distance(distance);
    if(__INIT__ == 0)
    {
      // Generate random starting point
      origin[0] = vec2(random(8.)/7., random(8.)/7.);
      // Generate random starting direction between [-1, -1] and [1, 1]
      vec2 dir = vec2(-1. + random(200)/200., -1. + random(200)/200.);
      direction[0] = dir;
      for(int l=0; l < snake_length-1; l++)
        origin[l+1] = origin[l].add(dir);
      __INIT__ = 1;
    }else
    {
      vec2 o = origin[0];
      vec2 d = direction[0];

      for(int l=0; l < snake_length; l++)
        trigger_pixel(origin[l], l==0?5 + 100*p:5);
      
      vec2 no = o.add(d);
      if(no.x <= 0 || no.x >= 7) 
      {
        d.x *= -1;
        d.y += .2 * abs(d.x)/d.x;
      }
      if(no.y <= 0 || no.y >= 7) 
      {
        d.y *= -1;
        d.x += .2 * abs(d.y)/d.y;
      }
      direction[0] = d;
      origin[3]=origin[2];
      origin[2]=origin[1];
      origin[1]=origin[0];
      for(int l=snake_length; l > 1; l--)
      {
          origin[l-1]=origin[l-2];
      }
      origin[0] = no;
      // Serial.print(d.x);
      // Serial.print(", ");
      // Serial.println(d.y);
      // Serial.print(o.x);
      // Serial.print(", ");
      // Serial.println(o.y);
      // Serial.println("");
      // Serial.println("");
    }
}

void loop()
{
  
  float distance = get_distance();
  snake(distance);
  // draw_ball(distance);
  // visualize(distance);
  
  // Serial.println(" cm");
}