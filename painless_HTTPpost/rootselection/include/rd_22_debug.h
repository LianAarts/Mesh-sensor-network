#if DEBUG >= 1
#define debug1(x) Serial.print(x)
#define debugln1(x) Serial.println(x)
#else
#define debug1(x)
#define debugln1(x)
#endif

#if DEBUG >= 2
#define debug2(x) Serial.print(x)
#define debugln2(x) Serial.println(x)
#else
#define debug2(x)
#define debugln2(x)
#endif

#if DEBUG >= 3
#define debug3(x) Serial.print(x)
#define debugln3(x) Serial.println(x)
#else
#define debug3(x)
#define debugln3(x)
#endif