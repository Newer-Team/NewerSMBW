#include <game.h>

const char *ts1table[][43] =
{
	"Pa1_chika",
	"Pa1_gake",
	"Pa1_nohara",
	"Pa1_sabaku",
	"Pa1_setsugen",
	"Pa1_shiro",
	"Pa1_suichu",
	"Pa1_kaigan",
	"Pa1_toride",
	"Pa1_toride_yogan",
	"Pa1_toride_sabaku",
	"Pa1_daishizen",
	"Pa1_obake",
	"Pa1_obake_soto",
	"Pa1_dokan_naibu",
	"Pa1_sabaku_chika",
	"Pa1_korichika",
	"Pa1_nohara2",
	"Pa1_kurayami_chika",
	"Pa1_shiro_yogan",
	"Pa1_koopa_out",
	"Pa1_MG_house",
	"Pa1_hikousen",
	"Pa1_hikousen2",
	"Pa1_toride_boss1",
	"Pa1_toride_boss2",
	"Pa1_toride_boss3",
	"Pa1_toride_boss4",
	"Pa1_toride_boss5",
	"Pa1_toride_boss6",
	"Pa1_toride_boss7",
	"Pa1_toride_boss8",
	"Pa1_shiro_boss1",
	"Pa1_peach_castle",
	"Pa1_toride_soto",
	"Pa1_shiro_sora",
	"Pa1_KinokoHouse",
	"Pa1_toride_kori",
	"Pa1_shiro_koopa",
	"Pa1_gake_yougan",
	// ADDITIONS
	"Pa1_daishizenkuri",
	"Pa1_e3setsugen",
	"Pa1_tilingshirosora"
};

const char *ts2table[][23] =
{
	"Pa2_kori",
	"Pa2_kinoko",
	"Pa2_doukutu",
	"Pa2_dokan_naibu",
	"Pa2_doukutu2",
	"Pa2_shiro_koopa",
	"Pa2_gake",
	"Pa2_sora",
	"Pa2_hikousen",
	"Pa2_hikousen2",
	"Pa2_doukutu3",
	"Pa2_doukutu4",
	"Pa2_doukutu5",
	"Pa2_doukutu6",
	"Pa2_doukutu7",
	"Pa2_doukutu8",
	// ADDITIONS
	"Pa2_daishizenkuri",
	"Pa2_undercastle",
	"Pa2_shiro",
	"Pa2_chika_0606",
	"Pa2_e3kori",
	"Pa2_forestobake",
	"Pa2_mountainpa2"
};

const char *ts3table[][10] =
{
	"Pa3_rail",
	"Pa3_rail_white",
	"Pa3_hanatari_saka",
	"Pa3_MG_house_ami_rail",
	"Pa3_shiro_koopa",
	"Pa3_daishizen",
	// ADDITIONS
	"Pa3_superplayer",
	"Pa3_gakebg",
	"Pa3_douk4sw",
	"Pa3_shroomrocks"
};

typedef struct PTMF {
    int classOffset;
    int vtableOffset;
    void (*func_addr)(TilemapClass*, BGRender*, int);
} PTMF;

PTMF Pa1RandomTiles[43] = {
    {0, -1, randomiseNormalTerrain},			// Pa1_chika
	{0, -1, randomiseGake},				// Pa1_gake
	{0, -1, randomiseNohara},			// Pa1_nohara
	{0, -1, randomiseSabaku},			// Pa1_sabaku
	{0, -1, randomiseNormalTerrain},		// Pa1_setsugen
	{0, -1, randomiseNormalTerrain},		// Pa1_shiro
	{0, -1, randomiseNormalTerrain},		// Pa1_suichu
	{0, -1, randomiseKaigan},			// Pa1_kaigan
	{0, 0, 0},					// Pa1_toride
	{0, 0, 0},					// Pa1_toride_yogan
	{0, 0, 0},					// Pa1_toride_sabaku
	{0, -1, randomiseNormalTerrain},		// Pa1_daishizen
	{0, -1, randomiseObake},			// Pa1_obake
	{0, -1, randomiseObakeSoto},			// Pa1_obake_soto
	{0, 0, 0},					// Pa1_dokan_naibu
	{0, -1, randomiseNormalTerrain},		// Pa1_sabaku_chika
	{0, -1, randomiseKoriChika},			// Pa1_korichika
	{0, -1, randomiseNormalTerrain},		// Pa1_nohara2
	{0, -1, randomiseNormalTerrain},		// Pa1_kurayami_chika
	{0, -1, randomiseNormalTerrain},		// Pa1_shiro_yogan
	{0, -1, randomiseNormalTerrain},		// Pa1_koopa_out
	{0, 0, 0},					// Pa1_mg_house
	{0, 0, 0},					// Pa1_hikousen
	{0, 0, 0},					// Pa1_hikousen2
	{0, 0, 0},					// Pa1_toride_boss1
	{0, 0, 0},					// Pa1_toride_boss2
	{0, 0, 0},					// Pa1_toride_boss3
	{0, 0, 0},					// Pa1_toride_boss4
	{0, 0, 0},					// Pa1_toride_boss5
	{0, 0, 0},					// Pa1_toride_boss6
	{0, 0, 0},					// Pa1_toride_boss7
	{0, 0, 0},					// Pa1_toride_boss8
	{0, -1, randomiseShiroBoss1},			// Pa1_shiro_boss1
	{0, 0, 0},					// Pa1_peach_castle
	{0, 0, 0},					// Pa1_toride_soto
	{0, -1, randomiseNormalTerrain},		// Pa1_shiro_sora			originally no randomisation
	{0, 0, 0},					// Pa1_KinokoHouse
	{0, 0, 0},					// Pa1_toride_kori
	{0, -1, randomiseNormalTerrain},		// Pa1_shiro_koopa
	{0, -1, randomiseNormalTerrain},		// Pa1_gake_yougan
	// ADDITIONS
	{0, -1, randomiseNormalTerrain},		// Pa1_daishizenkuri
	{0, -1, randomiseNormalTerrain},		// Pa1_e3setsugen
	{0, -1, randomiseNormalTerrain}			// Pa1_tilingshirosora
};

PTMF Pa2RandomTiles[23] = {
	{0, -1, randomisePa2Kori},			// Pa2_kori
	{0, 0, 0},					// Pa2_kinoko
	{0, -1, randomiseDoukutu},			// Pa2_doukutu
	{0, 0, 0},					// Pa2_dokan_naibu
	{0, -1, randomiseDoukutu},			// Pa2_doukutu2
	{0, 0, 0},					// Pa2_shiro_koopa
	{0, 0, 0},					// Pa2_gake
	{0, 0, 0},					// Pa2_sora
	{0, 0, 0},					// Pa2_hikousen
	{0, 0, 0},					// Pa2_hikousen2
	{0, -1, randomiseNormalTerrain},		// Pa2_doukutu3
	{0, -1, randomiseNormalTerrain},		// Pa2_doukutu4
	{0, -1, randomiseNormalTerrain},		// Pa2_doukutu5
	{0, -1, randomiseNormalTerrain},		// Pa2_doukutu6
	{0, 0, 0},					// Pa2_doukutu7
	{0, -1, randomiseNormalTerrain},		// Pa2_doukutu8
	// ADDITIONS
	{0, -1, randomiseNormalTerrain},		// Pa2_daishizenkuri
	{0, -1, randomiseNormalTerrain},		// Pa2_undercastle
	{0, -1, randomiseNormalTerrain},		// Pa2_shiro
	{0, -1, randomiseNormalTerrain},		// Pa3_chika_0606
	{0, -1, randomisePa2Kori},			// Pa2_e3kori
	{0, -1, randomiseObakeSoto},			// Pa2_forestobake
	{0, -1, randomiseGake}				// Pa2_mountainpa2
};

PTMF Pa3RandomTiles[10] = {
	{0, 0, 0},					// Pa3_rail
	{0, 0, 0},					// Pa3_rail_white
	{0, 0, 0},					// Pa3_hanatari_saka
	{0, 0, 0},					// Pa3_MG_house_ami_rail
	{0, 0, 0},					// Pa3_shiro_koopa
	{0, 0, 0},					// Pa3_daishizen
	// ADDITIONS
	{0, -1, randomiseNormalTerrain},		// Pa3_superplayer
	{0, -1, randomiseGake},				// Pa3_gakebg
	{0, -1, randomiseNormalTerrain},		// Pa3_douk4sw
	{0, -1, randomiseNormalTerrain}			// Pa3_shroomrocks
};
