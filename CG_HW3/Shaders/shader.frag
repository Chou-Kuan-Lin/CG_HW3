//fragment
#version 400

// model information
in vec2 modelTexcoord;
in vec3 modelPosition;

//texture
out vec4 finalColor;	// 最後輸出色
uniform sampler2D mainTexture;	// 網格紋理
uniform float textureRepeatX;
uniform float textureRepeatY;
uniform sampler2D depthTexture;	// texture的深度資訊
uniform float textureWidth;
uniform float textureHeight;

// Pulsing Effect
uniform float pulsingClock;
float pulsingFunction(float y, float clock) {
	// just like sin function
	float position = abs( 1 - fract(clock) );	// Top - clock
	float differ = abs( abs(y) - position );	// clock - y(scan)
	float width = 0.111;

	if( differ >= width ) 
		return 1;	// black color
	else 
		return differ/width;	// band boundary
}

// Rim Effect
uniform vec3 cameraPosition;
in vec3 rimPosition;
in vec3 rimNormal;
float rimFunction( vec3 normal, vec3 viewDir ) {
	float dotValue = dot( normal, viewDir );
	if( abs( dotValue ) < 0.2 )
		return 1 - 2 * abs( dotValue );
	else
		return 0;
}

// Pole Highlight
float poleFunction(float y) {
	float coeff = 1 - 1 * ( abs(1 - y) );
	coeff = max( coeff, 0 );
	return coeff;
}

// Intersection Highlight
float linearize(float d)
{
	// gluPerspective
	// math formula
	float near = 0.001;
	float far = 100;
	return 12 * (2 * near) / ( far + near - (d * 2 - 1) * (far - near) );
}

float checkIntersect(float n)
{
	if( abs( linearize(gl_FragCoord.z) - n ) <0.003 )
		return 1;
	else
		return 0;
}

void main()
{
	// Common Information
	// Color
	vec4 blueColor = vec4(16, 59, 159, 4) / 255.0;
	vec4 whiteColor = vec4(1,1,1,1);

	// Texture
	vec2 texcoord = vec2( modelTexcoord.x * textureRepeatX, modelTexcoord.y * textureRepeatY );	// 細化網格
	vec4 texture = texture2D( mainTexture, texcoord );
	
	// Pulsing Effect
	float multiplier = pulsingFunction( modelPosition.y, pulsingClock );
	vec4 pulsingColor = multiplier * texture * blueColor;

	// Rim Effect
	vec3 viewDir = normalize( cameraPosition - rimPosition );
	float r = rimFunction( rimNormal, viewDir );
	vec4 rimColor = vec4( r, r, r, 1 );
	
	// Pole Highlight
	float p = poleFunction( modelPosition.y );
	vec4 poleColor = vec4( p, p, p, 1 );

	// Intersect Highlight
	vec2 coordinate = vec2( (gl_FragCoord.x/textureHeight), (gl_FragCoord.y/textureWidth) );	// 規格化[0,1]
	float raw = texture2D( depthTexture, coordinate ).b;	// R = G = B = depth value
	float isIntersect = checkIntersect(linearize(raw));
	vec4 intersectColor = vec4(isIntersect, isIntersect, isIntersect, 1);
	
	// Result Color Compute
	vec4 glow = max( max( intersectColor,rimColor ), poleColor );
	vec4 glowColor = mix( blueColor, whiteColor, glow );

	// finalColor = pulsingColor + texture * blueColor;
	finalColor = pulsingColor + ( poleColor + rimColor + intersectColor ) * glowColor;
	finalColor.a = 0.777;
}