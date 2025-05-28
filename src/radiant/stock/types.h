
#define QER_TRANS     0x00000001
#define QER_NOCARVE   0x00000002
#define MAX_POINTS_ON_WINDING 64
#define	MAX_PATCH_WIDTH		16
#define	MAX_PATCH_HEIGHT	16
#define MAX_TERRAIN_TEXTURES 128

typedef unsigned char byte;
typedef enum { qfalse, qtrue } qboolean;
typedef int qhandle_t;
typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

class texdef_t
{
public:
	texdef_t()
	{
		name = new char[1];
		name[0] = '\0';
	}
	~texdef_t()
	{
		delete []name;
		name = NULL;
	}

	const char *Name( void )
	{
		if ( name ) {
			return name;
		}

		return "";
	}

	void SetName(const char *p)
	{
		if (name)
		{
		delete []name;
		}
		if (p)
		{
		name = strcpy(new char[strlen(p)+1], p);
		}
		else
		{
		name = new char[1];
		name[0] = '\0';
		}
	}

	texdef_t& operator =(const texdef_t& rhs)
	{
		if (&rhs != this)
		{
		SetName(rhs.name);
		shift[0] = rhs.shift[0];
		shift[1] = rhs.shift[1];
		rotate = rhs.rotate;
		scale[0] = rhs.scale[0];
		scale[1] = rhs.scale[1];
		contents = rhs.contents;
		flags = rhs.flags;
		value = rhs.value;
		}
		return *this;
	}
		//char	name[128];
		char	*name;
		float	shift[2];
		float	rotate;
		float	scale[2];
		int		contents;
		int		flags;
		int		value;
};

typedef struct qtexture_s
{
	struct	qtexture_s *next;
	char	name[64];		// includes partial directory and extension
int		width,  height;
	int		contents;
	int		flags;
	int		value;
	int		texture_number;	// gl bind number

	// name of the .shader file
char  shadername[1024]; // old shader stuff
qboolean bFromShader;   // created from a shader
float fTrans;           // amount of transparency
int   nShaderFlags;     // qer_ shader flags
	vec3_t	color;			    // for flat shade mode
	qboolean	inuse;		    // true = is present on the level

	// cast this one to an IPluginQTexture if you are using it
	// NOTE: casting can be done with a GETPLUGINQTEXTURE defined in isurfaceplugin.h
	// TODO: if the __ISURFACEPLUGIN_H_ header is used, use a union { void *pData; IPluginQTexture *pPluginQTexture } kind of thing ?
	void					*pData;

	//++timo FIXME: this is the actual filename of the texture
	// this will be removed after shader code cleanup
	char filename[64];

} qtexture_t;

typedef struct
{
	int		numpoints;
	int		maxpoints;
	float 	points[8][5];			// variable sized
} winding_t;

typedef struct
{
	vec3_t	normal;
	double	dist;
	int		type;
} plane_t;

// Timo
// new brush primitive texdef
typedef struct brushprimit_texdef_s
{
	vec_t	coords[2][3];
} brushprimit_texdef_t;

//++timo texdef and brushprimit_texdef are static
// TODO : do dynamic ?
typedef struct face_s
{
	struct face_s			*next;
	struct face_s			*original;		//used for vertex movement
	vec3_t					planepts[3];
	texdef_t				texdef;
	plane_t					plane;

	winding_t				*face_winding;

	vec3_t					d_color;
	qtexture_t				*d_texture;

	// Timo new brush primit texdef
	brushprimit_texdef_t	brushprimit_texdef;

	// cast this one to an IPluginTexdef if you are using it
	// NOTE: casting can be done with a GETPLUGINTEXDEF defined in isurfaceplugin.h
	// TODO: if the __ISURFACEPLUGIN_H_ header is used, use a union { void *pData; IPluginTexdef *pPluginTexdef } kind of thing ?
	void					*pData;
} face_t;

// used in brush primitive AND entities
typedef struct epair_s
{
	struct epair_s	*next;
	char	*key;
	char	*value;
} epair_t;

struct brush_s;
typedef struct brush_s brush_t;

typedef struct
{
	vec3_t		xyz;
	float		st[2];
	float		lightmap[2];
	vec3_t		normal;
} drawVert_t;

typedef struct
{
	int				index;
	qtexture_t		*texture;
	texdef_t		texdef;
} terrainFace_t;

typedef struct
{
	float			height;
	float			scale;
	terrainFace_t	tri;
	vec4_t			rgba;
	vec3_t			normal;
	vec3_t			xyz;
} terrainVert_t;

typedef struct {
	int	width, height;		// in control points, not patches
	int   contents, flags, value, type;
	qtexture_t *d_texture;
	drawVert_t ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT];
	brush_t *pSymbiot;
	qboolean bSelected;
	qboolean bOverlay;
	qboolean bDirty;
	int  nListID;
	epair_t *epairs;
	// cast this one to an IPluginTexdef if you are using it
	// NOTE: casting can be done with a GETPLUGINTEXDEF defined in isurfaceplugin.h
	// TODO: if the __ISURFACEPLUGIN_H_ header is used, use a union { void *pData; IPluginTexdef *pPluginTexdef } kind of thing ?
	void					*pData;
} patchMesh_t;

typedef struct
{
	int				width, height;

	vec3_t			mins, maxs;
	vec3_t			origin;
	float			scale_x;
	float			scale_y;

	int				numtextures;
	qtexture_t		*textures[ MAX_TERRAIN_TEXTURES ];

	terrainVert_t	*heightmap;       // width * height

	epair_t			*epairs;

	brush_s			*pSymbiot;
	bool			bSelected;
	bool			bDirty;
	int				nListID;
} terrainMesh_t;

typedef struct brush_s
{
	struct brush_s	*prev, *next;	// links in active/selected
	struct brush_s	*oprev, *onext;	// links in entity
	struct entity_s	*owner;
	vec3_t	mins, maxs;
	face_t     *brush_faces;

	qboolean bModelFailed;
	//
	// curve brush extensions
	// all are derived from brush_faces
	qboolean	patchBrush;
	qboolean	hiddenBrush;
	qboolean	terrainBrush;

//int nPatchID;

	patchMesh_t *pPatch;
	terrainMesh_t	*pTerrain;

	struct entity_s *pUndoOwner;

	int undoId;						//undo ID
	int redoId;						//redo ID
	int ownerId;					//entityId of the owner entity for undo

	// TTimo: HTREEITEM is MFC, some plugins really don't like it
#ifdef QERTYPES_USE_MFC
	int numberId;         // brush number
	HTREEITEM itemOwner;  // owner for grouping
#else
	int numberId;
	DWORD itemOwner;
#endif

	// brush primitive only
	epair_t *epairs;

} brush_t;
