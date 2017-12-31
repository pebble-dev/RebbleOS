#include "common.h"
#include "rebbleos.h"
#include "librebble.h"

GBitmap *gbitmap_create_with_resource_proxy(uint32_t resource_id);
ResHandle *resource_get_handle_proxy(uint16_t resource_id);
GFont *fonts_load_custom_font_proxy(ResHandle *handle);
bool persist_exists(void);
bool persist_exists(void) { return false; }

typedef void (*VoidFunc)(void);

void unalloc1() { printf("UNALLOC 1\n"); }
void unalloc2() { printf("UNALLOC 2\n"); }
void unalloc3() { printf("UNALLOC 3\n"); }
void unalloc4() { printf("UNALLOC 4\n"); }
void unalloc5() { printf("UNALLOC 5\n"); }
void unalloc6() { printf("UNALLOC 6\n"); }
void unalloc7() { printf("UNALLOC 7\n"); }
void unalloc8() { printf("UNALLOC 8\n"); }
void unalloc9() { printf("UNALLOC 9\n"); }
void unalloc10() { printf("UNALLOC 10\n"); }
void unalloc11() { printf("UNALLOC 11\n"); }
void unalloc12() { printf("UNALLOC 12\n"); }
void unalloc13() { printf("UNALLOC 13\n"); }
void unalloc14() { printf("UNALLOC 14\n"); }
void unalloc15() { printf("UNALLOC 15\n"); }
void unalloc16() { printf("UNALLOC 16\n"); }
void unalloc17() { printf("UNALLOC 17\n"); }
void unalloc18() { printf("UNALLOC 18\n"); }
void unalloc19() { printf("UNALLOC 19\n"); }
void unalloc20() { printf("UNALLOC 20\n"); }
void unalloc21() { printf("UNALLOC 21\n"); }
void unalloc22() { printf("UNALLOC 22\n"); }
void unalloc23() { printf("UNALLOC 23\n"); }
void unalloc24() { printf("UNALLOC 24\n"); }
void unalloc25() { printf("UNALLOC 25\n"); }
void unalloc26() { printf("UNALLOC 26\n"); }
void unalloc27() { printf("UNALLOC 27\n"); }
void unalloc28() { printf("UNALLOC 28\n"); }
void unalloc29() { printf("UNALLOC 29\n"); }
void unalloc30() { printf("UNALLOC 30\n"); }
void unalloc31() { printf("UNALLOC 31\n"); }
void unalloc32() { printf("UNALLOC 32\n"); }
void unalloc33() { printf("UNALLOC 33\n"); }
void unalloc34() { printf("UNALLOC 34\n"); }
void unalloc35() { printf("UNALLOC 35\n"); }
void unalloc36() { printf("UNALLOC 36\n"); }
void unalloc37() { printf("UNALLOC 37\n"); }
void unalloc38() { printf("UNALLOC 38\n"); }
void unalloc39() { printf("UNALLOC 39\n"); }
void unalloc40() { printf("UNALLOC 40\n"); }
void unalloc41() { printf("UNALLOC 41\n"); }
void unalloc42() { printf("UNALLOC 42\n"); }
void unalloc43() { printf("UNALLOC 43\n"); }
void unalloc44() { printf("UNALLOC 44\n"); }
void unalloc45() { printf("UNALLOC 45\n"); }
void unalloc46() { printf("UNALLOC 46\n"); }
void unalloc47() { printf("UNALLOC 47\n"); }
void unalloc48() { printf("UNALLOC 48\n"); }
void unalloc49() { printf("UNALLOC 49\n"); }
void unalloc50() { printf("UNALLOC 50\n"); }
void unalloc51() { printf("UNALLOC 51\n"); }
void unalloc52() { printf("UNALLOC 52\n"); }
void unalloc53() { printf("UNALLOC 53\n"); }
void unalloc54() { printf("UNALLOC 54\n"); }
void unalloc55() { printf("UNALLOC 55\n"); }
void unalloc56() { printf("UNALLOC 56\n"); }
void unalloc57() { printf("UNALLOC 57\n"); }
void unalloc58() { printf("UNALLOC 58\n"); }
void unalloc59() { printf("UNALLOC 59\n"); }
void unalloc60() { printf("UNALLOC 60\n"); }
void unalloc61() { printf("UNALLOC 61\n"); }
void unalloc62() { printf("UNALLOC 62\n"); }
void unalloc63() { printf("UNALLOC 63\n"); }
void unalloc64() { printf("UNALLOC 64\n"); }
void unalloc65() { printf("UNALLOC 65\n"); }
void unalloc66() { printf("UNALLOC 66\n"); }
void unalloc67() { printf("UNALLOC 67\n"); }
void unalloc68() { printf("UNALLOC 68\n"); }
void unalloc69() { printf("UNALLOC 69\n"); }
void unalloc70() { printf("UNALLOC 70\n"); }
void unalloc71() { printf("UNALLOC 71\n"); }
void unalloc72() { printf("UNALLOC 72\n"); }
void unalloc73() { printf("UNALLOC 73\n"); }
void unalloc74() { printf("UNALLOC 74\n"); }
void unalloc75() { printf("UNALLOC 75\n"); }
void unalloc76() { printf("UNALLOC 76\n"); }
void unalloc77() { printf("UNALLOC 77\n"); }
void unalloc78() { printf("UNALLOC 78\n"); }
void unalloc79() { printf("UNALLOC 79\n"); }
void unalloc80() { printf("UNALLOC 80\n"); }
void unalloc81() { printf("UNALLOC 81\n"); }
void unalloc82() { printf("UNALLOC 82\n"); }
void unalloc83() { printf("UNALLOC 83\n"); }
void unalloc84() { printf("UNALLOC 84\n"); }
void unalloc85() { printf("UNALLOC 85\n"); }
void unalloc86() { printf("UNALLOC 86\n"); }
void unalloc87() { printf("UNALLOC 87\n"); }
void unalloc88() { printf("UNALLOC 88\n"); }
void unalloc89() { printf("UNALLOC 89\n"); }
void unalloc90() { printf("UNALLOC 90\n"); }
void unalloc91() { printf("UNALLOC 91\n"); }
void unalloc92() { printf("UNALLOC 92\n"); }
void unalloc93() { printf("UNALLOC 93\n"); }
void unalloc94() { printf("UNALLOC 94\n"); }
void unalloc95() { printf("UNALLOC 95\n"); }
void unalloc96() { printf("UNALLOC 96\n"); }
void unalloc97() { printf("UNALLOC 97\n"); }
void unalloc98() { printf("UNALLOC 98\n"); }
void unalloc99() { printf("UNALLOC 99\n"); }
void unalloc100() { printf("UNALLOC 100\n"); }
void unalloc101() { printf("UNALLOC 101\n"); }
void unalloc102() { printf("UNALLOC 102\n"); }
void unalloc103() { printf("UNALLOC 103\n"); }
void unalloc104() { printf("UNALLOC 104\n"); }
void unalloc105() { printf("UNALLOC 105\n"); }
void unalloc106() { printf("UNALLOC 106\n"); }
void unalloc107() { printf("UNALLOC 107\n"); }
void unalloc108() { printf("UNALLOC 108\n"); }
void unalloc109() { printf("UNALLOC 109\n"); }
void unalloc110() { printf("UNALLOC 110\n"); }
void unalloc111() { printf("UNALLOC 111\n"); }
void unalloc112() { printf("UNALLOC 112\n"); }
void unalloc113() { printf("UNALLOC 113\n"); }
void unalloc114() { printf("UNALLOC 114\n"); }
void unalloc115() { printf("UNALLOC 115\n"); }
void unalloc116() { printf("UNALLOC 116\n"); }
void unalloc117() { printf("UNALLOC 117\n"); }
void unalloc118() { printf("UNALLOC 118\n"); }
void unalloc119() { printf("UNALLOC 119\n"); }
void unalloc120() { printf("UNALLOC 120\n"); }
void unalloc121() { printf("UNALLOC 121\n"); }
void unalloc122() { printf("UNALLOC 122\n"); }
void unalloc123() { printf("UNALLOC 123\n"); }
void unalloc124() { printf("UNALLOC 124\n"); }
void unalloc125() { printf("UNALLOC 125\n"); }
void unalloc126() { printf("UNALLOC 126\n"); }
void unalloc127() { printf("UNALLOC 127\n"); }
void unalloc128() { printf("UNALLOC 128\n"); }
void unalloc129() { printf("UNALLOC 129\n"); }
void unalloc130() { printf("UNALLOC 130\n"); }
void unalloc131() { printf("UNALLOC 131\n"); }
void unalloc132() { printf("UNALLOC 132\n"); }
void unalloc133() { printf("UNALLOC 133\n"); }
void unalloc134() { printf("UNALLOC 134\n"); }
void unalloc135() { printf("UNALLOC 135\n"); }
void unalloc136() { printf("UNALLOC 136\n"); }
void unalloc137() { printf("UNALLOC 137\n"); }
void unalloc138() { printf("UNALLOC 138\n"); }
void unalloc139() { printf("UNALLOC 139\n"); }
void unalloc140() { printf("UNALLOC 140\n"); }
void unalloc141() { printf("UNALLOC 141\n"); }
void unalloc142() { printf("UNALLOC 142\n"); }
void unalloc143() { printf("UNALLOC 143\n"); }
void unalloc144() { printf("UNALLOC 144\n"); }
void unalloc145() { printf("UNALLOC 145\n"); }
void unalloc146() { printf("UNALLOC 146\n"); }
void unalloc147() { printf("UNALLOC 147\n"); }
void unalloc148() { printf("UNALLOC 148\n"); }
void unalloc149() { printf("UNALLOC 149\n"); }
void unalloc150() { printf("UNALLOC 150\n"); }
void unalloc151() { printf("UNALLOC 151\n"); }
void unalloc152() { printf("UNALLOC 152\n"); }
void unalloc153() { printf("UNALLOC 153\n"); }
void unalloc154() { printf("UNALLOC 154\n"); }
void unalloc155() { printf("UNALLOC 155\n"); }
void unalloc156() { printf("UNALLOC 156\n"); }
void unalloc157() { printf("UNALLOC 157\n"); }
void unalloc158() { printf("UNALLOC 158\n"); }
void unalloc159() { printf("UNALLOC 159\n"); }
void unalloc160() { printf("UNALLOC 160\n"); }
void unalloc161() { printf("UNALLOC 161\n"); }
void unalloc162() { printf("UNALLOC 162\n"); }
void unalloc163() { printf("UNALLOC 163\n"); }
void unalloc164() { printf("UNALLOC 164\n"); }
void unalloc165() { printf("UNALLOC 165\n"); }
void unalloc166() { printf("UNALLOC 166\n"); }
void unalloc167() { printf("UNALLOC 167\n"); }
void unalloc168() { printf("UNALLOC 168\n"); }
void unalloc169() { printf("UNALLOC 169\n"); }
void unalloc170() { printf("UNALLOC 170\n"); }
void unalloc171() { printf("UNALLOC 171\n"); }
void unalloc172() { printf("UNALLOC 172\n"); }
void unalloc173() { printf("UNALLOC 173\n"); }
void unalloc174() { printf("UNALLOC 174\n"); }
void unalloc175() { printf("UNALLOC 175\n"); }
void unalloc176() { printf("UNALLOC 176\n"); }
void unalloc177() { printf("UNALLOC 177\n"); }
void unalloc178() { printf("UNALLOC 178\n"); }
void unalloc179() { printf("UNALLOC 179\n"); }
void unalloc180() { printf("UNALLOC 180\n"); }
void unalloc181() { printf("UNALLOC 181\n"); }
void unalloc182() { printf("UNALLOC 182\n"); }
void unalloc183() { printf("UNALLOC 183\n"); }
void unalloc184() { printf("UNALLOC 184\n"); }
void unalloc185() { printf("UNALLOC 185\n"); }
void unalloc186() { printf("UNALLOC 186\n"); }
void unalloc187() { printf("UNALLOC 187\n"); }
void unalloc188() { printf("UNALLOC 188\n"); }
void unalloc189() { printf("UNALLOC 189\n"); }
void unalloc190() { printf("UNALLOC 190\n"); }
void unalloc191() { printf("UNALLOC 191\n"); }
void unalloc192() { printf("UNALLOC 192\n"); }
void unalloc193() { printf("UNALLOC 193\n"); }
void unalloc194() { printf("UNALLOC 194\n"); }
void unalloc195() { printf("UNALLOC 195\n"); }
void unalloc196() { printf("UNALLOC 196\n"); }
void unalloc197() { printf("UNALLOC 197\n"); }
void unalloc198() { printf("UNALLOC 198\n"); }
void unalloc199() { printf("UNALLOC 199\n"); }
void unalloc200() { printf("UNALLOC 200\n"); }
void unalloc201() { printf("UNALLOC 201\n"); }
void unalloc202() { printf("UNALLOC 202\n"); }
void unalloc203() { printf("UNALLOC 203\n"); }
void unalloc204() { printf("UNALLOC 204\n"); }
void unalloc205() { printf("UNALLOC 205\n"); }
void unalloc206() { printf("UNALLOC 206\n"); }
void unalloc207() { printf("UNALLOC 207\n"); }
void unalloc208() { printf("UNALLOC 208\n"); }
void unalloc209() { printf("UNALLOC 209\n"); }
void unalloc210() { printf("UNALLOC 210\n"); }
void unalloc211() { printf("UNALLOC 211\n"); }
void unalloc212() { printf("UNALLOC 212\n"); }
void unalloc213() { printf("UNALLOC 213\n"); }
void unalloc214() { printf("UNALLOC 214\n"); }
void unalloc215() { printf("UNALLOC 215\n"); }
void unalloc216() { printf("UNALLOC 216\n"); }
void unalloc217() { printf("UNALLOC 217\n"); }
void unalloc218() { printf("UNALLOC 218\n"); }
void unalloc219() { printf("UNALLOC 219\n"); }
void unalloc220() { printf("UNALLOC 220\n"); }
void unalloc221() { printf("UNALLOC 221\n"); }
void unalloc222() { printf("UNALLOC 222\n"); }
void unalloc223() { printf("UNALLOC 223\n"); }
void unalloc224() { printf("UNALLOC 224\n"); }
void unalloc225() { printf("UNALLOC 225\n"); }
void unalloc226() { printf("UNALLOC 226\n"); }
void unalloc227() { printf("UNALLOC 227\n"); }
void unalloc228() { printf("UNALLOC 228\n"); }
void unalloc229() { printf("UNALLOC 229\n"); }
void unalloc230() { printf("UNALLOC 230\n"); }
void unalloc231() { printf("UNALLOC 231\n"); }
void unalloc232() { printf("UNALLOC 232\n"); }
void unalloc233() { printf("UNALLOC 233\n"); }
void unalloc234() { printf("UNALLOC 234\n"); }
void unalloc235() { printf("UNALLOC 235\n"); }
void unalloc236() { printf("UNALLOC 236\n"); }
void unalloc237() { printf("UNALLOC 237\n"); }
void unalloc238() { printf("UNALLOC 238\n"); }
void unalloc239() { printf("UNALLOC 239\n"); }
void unalloc240() { printf("UNALLOC 240\n"); }
void unalloc241() { printf("UNALLOC 241\n"); }
void unalloc242() { printf("UNALLOC 242\n"); }
void unalloc243() { printf("UNALLOC 243\n"); }
void unalloc244() { printf("UNALLOC 244\n"); }
void unalloc245() { printf("UNALLOC 245\n"); }
void unalloc246() { printf("UNALLOC 246\n"); }
void unalloc247() { printf("UNALLOC 247\n"); }
void unalloc248() { printf("UNALLOC 248\n"); }
void unalloc249() { printf("UNALLOC 249\n"); }
void unalloc250() { printf("UNALLOC 250\n"); }
void unalloc251() { printf("UNALLOC 251\n"); }
void unalloc252() { printf("UNALLOC 252\n"); }
void unalloc253() { printf("UNALLOC 253\n"); }
void unalloc254() { printf("UNALLOC 254\n"); }
void unalloc255() { printf("UNALLOC 255\n"); }
void unalloc256() { printf("UNALLOC 256\n"); }
void unalloc257() { printf("UNALLOC 257\n"); }
void unalloc258() { printf("UNALLOC 258\n"); }
void unalloc259() { printf("UNALLOC 259\n"); }
void unalloc260() { printf("UNALLOC 260\n"); }
void unalloc261() { printf("UNALLOC 261\n"); }
void unalloc262() { printf("UNALLOC 262\n"); }
void unalloc263() { printf("UNALLOC 263\n"); }
void unalloc264() { printf("UNALLOC 264\n"); }
void unalloc265() { printf("UNALLOC 265\n"); }
void unalloc266() { printf("UNALLOC 266\n"); }
void unalloc267() { printf("UNALLOC 267\n"); }
void unalloc268() { printf("UNALLOC 268\n"); }
void unalloc269() { printf("UNALLOC 269\n"); }
void unalloc270() { printf("UNALLOC 270\n"); }
void unalloc271() { printf("UNALLOC 271\n"); }
void unalloc272() { printf("UNALLOC 272\n"); }
void unalloc273() { printf("UNALLOC 273\n"); }
void unalloc274() { printf("UNALLOC 274\n"); }
void unalloc275() { printf("UNALLOC 275\n"); }
void unalloc276() { printf("UNALLOC 276\n"); }
void unalloc277() { printf("UNALLOC 277\n"); }
void unalloc278() { printf("UNALLOC 278\n"); }
void unalloc279() { printf("UNALLOC 279\n"); }
void unalloc280() { printf("UNALLOC 280\n"); }
void unalloc281() { printf("UNALLOC 281\n"); }
void unalloc282() { printf("UNALLOC 282\n"); }
void unalloc283() { printf("UNALLOC 283\n"); }
void unalloc284() { printf("UNALLOC 284\n"); }
void unalloc285() { printf("UNALLOC 285\n"); }
void unalloc286() { printf("UNALLOC 286\n"); }
void unalloc287() { printf("UNALLOC 287\n"); }
void unalloc288() { printf("UNALLOC 288\n"); }
void unalloc289() { printf("UNALLOC 289\n"); }
void unalloc290() { printf("UNALLOC 290\n"); }
void unalloc291() { printf("UNALLOC 291\n"); }
void unalloc292() { printf("UNALLOC 292\n"); }
void unalloc293() { printf("UNALLOC 293\n"); }
void unalloc294() { printf("UNALLOC 294\n"); }
void unalloc295() { printf("UNALLOC 295\n"); }
void unalloc296() { printf("UNALLOC 296\n"); }
void unalloc297() { printf("UNALLOC 297\n"); }
void unalloc298() { printf("UNALLOC 298\n"); }
void unalloc299() { printf("UNALLOC 299\n"); }
void unalloc300() { printf("UNALLOC 300\n"); }
void unalloc301() { printf("UNALLOC 301\n"); }
void unalloc302() { printf("UNALLOC 302\n"); }
void unalloc303() { printf("UNALLOC 303\n"); }
void unalloc304() { printf("UNALLOC 304\n"); }
void unalloc305() { printf("UNALLOC 305\n"); }
void unalloc306() { printf("UNALLOC 306\n"); }
void unalloc307() { printf("UNALLOC 307\n"); }
void unalloc308() { printf("UNALLOC 308\n"); }
void unalloc309() { printf("UNALLOC 309\n"); }
void unalloc310() { printf("UNALLOC 310\n"); }
void unalloc311() { printf("UNALLOC 311\n"); }
void unalloc312() { printf("UNALLOC 312\n"); }
void unalloc313() { printf("UNALLOC 313\n"); }
void unalloc314() { printf("UNALLOC 314\n"); }
void unalloc315() { printf("UNALLOC 315\n"); }
void unalloc316() { printf("UNALLOC 316\n"); }
void unalloc317() { printf("UNALLOC 317\n"); }
void unalloc318() { printf("UNALLOC 318\n"); }
void unalloc319() { printf("UNALLOC 319\n"); }
void unalloc320() { printf("UNALLOC 320\n"); }
void unalloc321() { printf("UNALLOC 321\n"); }
void unalloc322() { printf("UNALLOC 322\n"); }
void unalloc323() { printf("UNALLOC 323\n"); }
void unalloc324() { printf("UNALLOC 324\n"); }
void unalloc325() { printf("UNALLOC 325\n"); }
void unalloc326() { printf("UNALLOC 326\n"); }
void unalloc327() { printf("UNALLOC 327\n"); }
void unalloc328() { printf("UNALLOC 328\n"); }
void unalloc329() { printf("UNALLOC 329\n"); }
void unalloc330() { printf("UNALLOC 330\n"); }
void unalloc331() { printf("UNALLOC 331\n"); }
void unalloc332() { printf("UNALLOC 332\n"); }
void unalloc333() { printf("UNALLOC 333\n"); }
void unalloc334() { printf("UNALLOC 334\n"); }
void unalloc335() { printf("UNALLOC 335\n"); }
void unalloc336() { printf("UNALLOC 336\n"); }
void unalloc337() { printf("UNALLOC 337\n"); }
void unalloc338() { printf("UNALLOC 338\n"); }
void unalloc339() { printf("UNALLOC 339\n"); }
void unalloc340() { printf("UNALLOC 340\n"); }
void unalloc341() { printf("UNALLOC 341\n"); }
void unalloc342() { printf("UNALLOC 342\n"); }
void unalloc343() { printf("UNALLOC 343\n"); }
void unalloc344() { printf("UNALLOC 344\n"); }
void unalloc345() { printf("UNALLOC 345\n"); }
void unalloc346() { printf("UNALLOC 346\n"); }
void unalloc347() { printf("UNALLOC 347\n"); }
void unalloc348() { printf("UNALLOC 348\n"); }
void unalloc349() { printf("UNALLOC 349\n"); }
void unalloc350() { printf("UNALLOC 350\n"); }
void unalloc351() { printf("UNALLOC 351\n"); }
void unalloc352() { printf("UNALLOC 352\n"); }
void unalloc353() { printf("UNALLOC 353\n"); }
void unalloc354() { printf("UNALLOC 354\n"); }
void unalloc355() { printf("UNALLOC 355\n"); }
void unalloc356() { printf("UNALLOC 356\n"); }
void unalloc357() { printf("UNALLOC 357\n"); }
void unalloc358() { printf("UNALLOC 358\n"); }
void unalloc359() { printf("UNALLOC 359\n"); }
void unalloc360() { printf("UNALLOC 360\n"); }
void unalloc361() { printf("UNALLOC 361\n"); }
void unalloc362() { printf("UNALLOC 362\n"); }
void unalloc363() { printf("UNALLOC 363\n"); }
void unalloc364() { printf("UNALLOC 364\n"); }
void unalloc365() { printf("UNALLOC 365\n"); }
void unalloc366() { printf("UNALLOC 366\n"); }
void unalloc367() { printf("UNALLOC 367\n"); }
void unalloc368() { printf("UNALLOC 368\n"); }
void unalloc369() { printf("UNALLOC 369\n"); }
void unalloc370() { printf("UNALLOC 370\n"); }
void unalloc371() { printf("UNALLOC 371\n"); }
void unalloc372() { printf("UNALLOC 372\n"); }
void unalloc373() { printf("UNALLOC 373\n"); }
void unalloc374() { printf("UNALLOC 374\n"); }
void unalloc375() { printf("UNALLOC 375\n"); }
void unalloc376() { printf("UNALLOC 376\n"); }
void unalloc377() { printf("UNALLOC 377\n"); }
void unalloc378() { printf("UNALLOC 378\n"); }
void unalloc379() { printf("UNALLOC 379\n"); }
void unalloc380() { printf("UNALLOC 380\n"); }
void unalloc381() { printf("UNALLOC 381\n"); }
void unalloc382() { printf("UNALLOC 382\n"); }
void unalloc383() { printf("UNALLOC 383\n"); }
void unalloc384() { printf("UNALLOC 384\n"); }
void unalloc385() { printf("UNALLOC 385\n"); }
void unalloc386() { printf("UNALLOC 386\n"); }
void unalloc387() { printf("UNALLOC 387\n"); }
void unalloc388() { printf("UNALLOC 388\n"); }
void unalloc389() { printf("UNALLOC 389\n"); }
void unalloc390() { printf("UNALLOC 390\n"); }
void unalloc391() { printf("UNALLOC 391\n"); }
void unalloc392() { printf("UNALLOC 392\n"); }
void unalloc393() { printf("UNALLOC 393\n"); }
void unalloc394() { printf("UNALLOC 394\n"); }
void unalloc395() { printf("UNALLOC 395\n"); }
void unalloc396() { printf("UNALLOC 396\n"); }
void unalloc397() { printf("UNALLOC 397\n"); }
void unalloc398() { printf("UNALLOC 398\n"); }
void unalloc399() { printf("UNALLOC 399\n"); }
void unalloc400() { printf("UNALLOC 400\n"); }
void unalloc401() { printf("UNALLOC 401\n"); }
void unalloc402() { printf("UNALLOC 402\n"); }
void unalloc403() { printf("UNALLOC 403\n"); }
void unalloc404() { printf("UNALLOC 404\n"); }
void unalloc405() { printf("UNALLOC 405\n"); }
void unalloc406() { printf("UNALLOC 406\n"); }
void unalloc407() { printf("UNALLOC 407\n"); }
void unalloc408() { printf("UNALLOC 408\n"); }
void unalloc409() { printf("UNALLOC 409\n"); }
void unalloc410() { printf("UNALLOC 410\n"); }
void unalloc411() { printf("UNALLOC 411\n"); }
void unalloc412() { printf("UNALLOC 412\n"); }
void unalloc413() { printf("UNALLOC 413\n"); }
void unalloc414() { printf("UNALLOC 414\n"); }
void unalloc415() { printf("UNALLOC 415\n"); }
void unalloc416() { printf("UNALLOC 416\n"); }
void unalloc417() { printf("UNALLOC 417\n"); }
void unalloc418() { printf("UNALLOC 418\n"); }
void unalloc419() { printf("UNALLOC 419\n"); }
void unalloc420() { printf("UNALLOC 420\n"); }
void unalloc421() { printf("UNALLOC 421\n"); }
void unalloc422() { printf("UNALLOC 422\n"); }
void unalloc423() { printf("UNALLOC 423\n"); }
void unalloc424() { printf("UNALLOC 424\n"); }
void unalloc425() { printf("UNALLOC 425\n"); }
void unalloc426() { printf("UNALLOC 426\n"); }
void unalloc427() { printf("UNALLOC 427\n"); }
void unalloc428() { printf("UNALLOC 428\n"); }
void unalloc429() { printf("UNALLOC 429\n"); }
void unalloc430() { printf("UNALLOC 430\n"); }
void unalloc431() { printf("UNALLOC 431\n"); }
void unalloc432() { printf("UNALLOC 432\n"); }
void unalloc433() { printf("UNALLOC 433\n"); }
void unalloc434() { printf("UNALLOC 434\n"); }
void unalloc435() { printf("UNALLOC 435\n"); }
void unalloc436() { printf("UNALLOC 436\n"); }
void unalloc437() { printf("UNALLOC 437\n"); }
void unalloc438() { printf("UNALLOC 438\n"); }
void unalloc439() { printf("UNALLOC 439\n"); }
void unalloc440() { printf("UNALLOC 440\n"); }
void unalloc441() { printf("UNALLOC 441\n"); }
void unalloc442() { printf("UNALLOC 442\n"); }
void unalloc443() { printf("UNALLOC 443\n"); }
void unalloc444() { printf("UNALLOC 444\n"); }
void unalloc445() { printf("UNALLOC 445\n"); }
void unalloc446() { printf("UNALLOC 446\n"); }
void unalloc447() { printf("UNALLOC 447\n"); }
void unalloc448() { printf("UNALLOC 448\n"); }
void unalloc449() { printf("UNALLOC 449\n"); }
void unalloc450() { printf("UNALLOC 450\n"); }
void unalloc451() { printf("UNALLOC 451\n"); }
void unalloc452() { printf("UNALLOC 452\n"); }
void unalloc453() { printf("UNALLOC 453\n"); }
void unalloc454() { printf("UNALLOC 454\n"); }
void unalloc455() { printf("UNALLOC 455\n"); }
void unalloc456() { printf("UNALLOC 456\n"); }
void unalloc457() { printf("UNALLOC 457\n"); }
void unalloc458() { printf("UNALLOC 458\n"); }
void unalloc459() { printf("UNALLOC 459\n"); }
void unalloc460() { printf("UNALLOC 460\n"); }
void unalloc461() { printf("UNALLOC 461\n"); }
void unalloc462() { printf("UNALLOC 462\n"); }
void unalloc463() { printf("UNALLOC 463\n"); }
void unalloc464() { printf("UNALLOC 464\n"); }
void unalloc465() { printf("UNALLOC 465\n"); }
void unalloc466() { printf("UNALLOC 466\n"); }
void unalloc467() { printf("UNALLOC 467\n"); }
void unalloc468() { printf("UNALLOC 468\n"); }
void unalloc469() { printf("UNALLOC 469\n"); }
void unalloc470() { printf("UNALLOC 470\n"); }
void unalloc471() { printf("UNALLOC 471\n"); }
void unalloc472() { printf("UNALLOC 472\n"); }
void unalloc473() { printf("UNALLOC 473\n"); }
void unalloc474() { printf("UNALLOC 474\n"); }
void unalloc475() { printf("UNALLOC 475\n"); }
void unalloc476() { printf("UNALLOC 476\n"); }
void unalloc477() { printf("UNALLOC 477\n"); }
void unalloc478() { printf("UNALLOC 478\n"); }
void unalloc479() { printf("UNALLOC 479\n"); }
void unalloc480() { printf("UNALLOC 480\n"); }
void unalloc481() { printf("UNALLOC 481\n"); }
void unalloc482() { printf("UNALLOC 482\n"); }
void unalloc483() { printf("UNALLOC 483\n"); }
void unalloc484() { printf("UNALLOC 484\n"); }
void unalloc485() { printf("UNALLOC 485\n"); }
void unalloc486() { printf("UNALLOC 486\n"); }
void unalloc487() { printf("UNALLOC 487\n"); }
void unalloc488() { printf("UNALLOC 488\n"); }
void unalloc489() { printf("UNALLOC 489\n"); }
void unalloc490() { printf("UNALLOC 490\n"); }
void unalloc491() { printf("UNALLOC 491\n"); }
void unalloc492() { printf("UNALLOC 492\n"); }
void unalloc493() { printf("UNALLOC 493\n"); }
void unalloc494() { printf("UNALLOC 494\n"); }
void unalloc495() { printf("UNALLOC 495\n"); }
void unalloc496() { printf("UNALLOC 496\n"); }
void unalloc497() { printf("UNALLOC 497\n"); }
void unalloc498() { printf("UNALLOC 498\n"); }
void unalloc499() { printf("UNALLOC 499\n"); }
void unalloc500() { printf("UNALLOC 500\n"); }
void unalloc501() { printf("UNALLOC 501\n"); }
void unalloc502() { printf("UNALLOC 502\n"); }
void unalloc503() { printf("UNALLOC 503\n"); }
void unalloc504() { printf("UNALLOC 504\n"); }
void unalloc505() { printf("UNALLOC 505\n"); }
void unalloc506() { printf("UNALLOC 506\n"); }
void unalloc507() { printf("UNALLOC 507\n"); }
void unalloc508() { printf("UNALLOC 508\n"); }
void unalloc509() { printf("UNALLOC 509\n"); }
void unalloc510() { printf("UNALLOC 510\n"); }
void unalloc511() { printf("UNALLOC 511\n"); }
void unalloc512() { printf("UNALLOC 512\n"); }
void unalloc513() { printf("UNALLOC 513\n"); }
void unalloc514() { printf("UNALLOC 514\n"); }
void unalloc515() { printf("UNALLOC 515\n"); }
void unalloc516() { printf("UNALLOC 516\n"); }
void unalloc517() { printf("UNALLOC 517\n"); }
void unalloc518() { printf("UNALLOC 518\n"); }
void unalloc519() { printf("UNALLOC 519\n"); }
void unalloc520() { printf("UNALLOC 520\n"); }
void unalloc521() { printf("UNALLOC 521\n"); }
void unalloc522() { printf("UNALLOC 522\n"); }
void unalloc523() { printf("UNALLOC 523\n"); }
void unalloc524() { printf("UNALLOC 524\n"); }
void unalloc525() { printf("UNALLOC 525\n"); }
void unalloc526() { printf("UNALLOC 526\n"); }
void unalloc527() { printf("UNALLOC 527\n"); }
void unalloc528() { printf("UNALLOC 528\n"); }
void unalloc529() { printf("UNALLOC 529\n"); }
void unalloc530() { printf("UNALLOC 530\n"); }
void unalloc531() { printf("UNALLOC 531\n"); }
void unalloc532() { printf("UNALLOC 532\n"); }
void unalloc533() { printf("UNALLOC 533\n"); }
void unalloc534() { printf("UNALLOC 534\n"); }
void unalloc535() { printf("UNALLOC 535\n"); }
void unalloc536() { printf("UNALLOC 536\n"); }
void unalloc537() { printf("UNALLOC 537\n"); }
void unalloc538() { printf("UNALLOC 538\n"); }
void unalloc539() { printf("UNALLOC 539\n"); }
void unalloc540() { printf("UNALLOC 540\n"); }
void unalloc541() { printf("UNALLOC 541\n"); }
void unalloc542() { printf("UNALLOC 542\n"); }
void unalloc543() { printf("UNALLOC 543\n"); }
void unalloc544() { printf("UNALLOC 544\n"); }
void unalloc545() { printf("UNALLOC 545\n"); }
void unalloc546() { printf("UNALLOC 546\n"); }
void unalloc547() { printf("UNALLOC 547\n"); }
void unalloc548() { printf("UNALLOC 548\n"); }
void unalloc549() { printf("UNALLOC 549\n"); }
void unalloc550() { printf("UNALLOC 550\n"); }
void unalloc551() { printf("UNALLOC 551\n"); }
void unalloc552() { printf("UNALLOC 552\n"); }
void unalloc553() { printf("UNALLOC 553\n"); }
void unalloc554() { printf("UNALLOC 554\n"); }
void unalloc555() { printf("UNALLOC 555\n"); }
void unalloc556() { printf("UNALLOC 556\n"); }
void unalloc557() { printf("UNALLOC 557\n"); }
void unalloc558() { printf("UNALLOC 558\n"); }
void unalloc559() { printf("UNALLOC 559\n"); }
void unalloc560() { printf("UNALLOC 560\n"); }
void unalloc561() { printf("UNALLOC 561\n"); }
void unalloc562() { printf("UNALLOC 562\n"); }
void unalloc563() { printf("UNALLOC 563\n"); }
void unalloc564() { printf("UNALLOC 564\n"); }
void unalloc565() { printf("UNALLOC 565\n"); }
void unalloc566() { printf("UNALLOC 566\n"); }
void unalloc567() { printf("UNALLOC 567\n"); }
void unalloc568() { printf("UNALLOC 568\n"); }
void unalloc569() { printf("UNALLOC 569\n"); }
void unalloc570() { printf("UNALLOC 570\n"); }
void unalloc571() { printf("UNALLOC 571\n"); }
void unalloc572() { printf("UNALLOC 572\n"); }
void unalloc573() { printf("UNALLOC 573\n"); }
void unalloc574() { printf("UNALLOC 574\n"); }
void unalloc575() { printf("UNALLOC 575\n"); }
void unalloc576() { printf("UNALLOC 576\n"); }
void unalloc577() { printf("UNALLOC 577\n"); }
void unalloc578() { printf("UNALLOC 578\n"); }
void unalloc579() { printf("UNALLOC 579\n"); }
void unalloc580() { printf("UNALLOC 580\n"); }
void unalloc581() { printf("UNALLOC 581\n"); }
void unalloc582() { printf("UNALLOC 582\n"); }
void unalloc583() { printf("UNALLOC 583\n"); }
void unalloc584() { printf("UNALLOC 584\n"); }
void unalloc585() { printf("UNALLOC 585\n"); }
void unalloc586() { printf("UNALLOC 586\n"); }
void unalloc587() { printf("UNALLOC 587\n"); }
void unalloc588() { printf("UNALLOC 588\n"); }
void unalloc589() { printf("UNALLOC 589\n"); }
void unalloc590() { printf("UNALLOC 590\n"); }
void unalloc591() { printf("UNALLOC 591\n"); }
void unalloc592() { printf("UNALLOC 592\n"); }
void unalloc593() { printf("UNALLOC 593\n"); }
void unalloc594() { printf("UNALLOC 594\n"); }
void unalloc595() { printf("UNALLOC 595\n"); }
void unalloc596() { printf("UNALLOC 596\n"); }
void unalloc597() { printf("UNALLOC 597\n"); }
void unalloc598() { printf("UNALLOC 598\n"); }
void unalloc599() { printf("UNALLOC 599\n"); }
void unalloc600() { printf("UNALLOC 600\n"); }
void unalloc601() { printf("UNALLOC 601\n"); }
void unalloc602() { printf("UNALLOC 602\n"); }
void unalloc603() { printf("UNALLOC 603\n"); }
void unalloc604() { printf("UNALLOC 604\n"); }
void unalloc605() { printf("UNALLOC 605\n"); }
void unalloc606() { printf("UNALLOC 606\n"); }
void unalloc607() { printf("UNALLOC 607\n"); }
void unalloc608() { printf("UNALLOC 608\n"); }
void unalloc609() { printf("UNALLOC 609\n"); }
void unalloc610() { printf("UNALLOC 610\n"); }
void unalloc611() { printf("UNALLOC 611\n"); }
void unalloc612() { printf("UNALLOC 612\n"); }
void unalloc613() { printf("UNALLOC 613\n"); }
void unalloc614() { printf("UNALLOC 614\n"); }
void unalloc615() { printf("UNALLOC 615\n"); }
void unalloc616() { printf("UNALLOC 616\n"); }
void unalloc617() { printf("UNALLOC 617\n"); }
void unalloc618() { printf("UNALLOC 618\n"); }
void unalloc619() { printf("UNALLOC 619\n"); }
void unalloc620() { printf("UNALLOC 620\n"); }
void unalloc621() { printf("UNALLOC 621\n"); }
void unalloc622() { printf("UNALLOC 622\n"); }
void unalloc623() { printf("UNALLOC 623\n"); }
void unalloc624() { printf("UNALLOC 624\n"); }
void unalloc625() { printf("UNALLOC 625\n"); }
void unalloc626() { printf("UNALLOC 626\n"); }
void unalloc627() { printf("UNALLOC 627\n"); }
void unalloc628() { printf("UNALLOC 628\n"); }
void unalloc629() { printf("UNALLOC 629\n"); }
void unalloc630() { printf("UNALLOC 630\n"); }
void unalloc631() { printf("UNALLOC 631\n"); }
void unalloc632() { printf("UNALLOC 632\n"); }
void unalloc633() { printf("UNALLOC 633\n"); }
void unalloc634() { printf("UNALLOC 634\n"); }
void unalloc635() { printf("UNALLOC 635\n"); }
void unalloc636() { printf("UNALLOC 636\n"); }
void unalloc637() { printf("UNALLOC 637\n"); }
void unalloc638() { printf("UNALLOC 638\n"); }
void unalloc639() { printf("UNALLOC 639\n"); }
void unalloc640() { printf("UNALLOC 640\n"); }
void unalloc641() { printf("UNALLOC 641\n"); }
void unalloc642() { printf("UNALLOC 642\n"); }
void unalloc643() { printf("UNALLOC 643\n"); }
void unalloc644() { printf("UNALLOC 644\n"); }
void unalloc645() { printf("UNALLOC 645\n"); }
void unalloc646() { printf("UNALLOC 646\n"); }
void unalloc647() { printf("UNALLOC 647\n"); }
void unalloc648() { printf("UNALLOC 648\n"); }
void unalloc649() { printf("UNALLOC 649\n"); }
void unalloc650() { printf("UNALLOC 650\n"); }
void unalloc651() { printf("UNALLOC 651\n"); }
void unalloc652() { printf("UNALLOC 652\n"); }
void unalloc653() { printf("UNALLOC 653\n"); }
void unalloc654() { printf("UNALLOC 654\n"); }
void unalloc655() { printf("UNALLOC 655\n"); }
void unalloc656() { printf("UNALLOC 656\n"); }
void unalloc657() { printf("UNALLOC 657\n"); }
void unalloc658() { printf("UNALLOC 658\n"); }
void unalloc659() { printf("UNALLOC 659\n"); }
void unalloc660() { printf("UNALLOC 660\n"); }
void unalloc661() { printf("UNALLOC 661\n"); }
void unalloc662() { printf("UNALLOC 662\n"); }
void unalloc663() { printf("UNALLOC 663\n"); }
void unalloc664() { printf("UNALLOC 664\n"); }
void unalloc665() { printf("UNALLOC 665\n"); }
void unalloc666() { printf("UNALLOC 666\n"); }
void unalloc667() { printf("UNALLOC 667\n"); }
void unalloc668() { printf("UNALLOC 668\n"); }
void unalloc669() { printf("UNALLOC 669\n"); }
void unalloc670() { printf("UNALLOC 670\n"); }
void unalloc671() { printf("UNALLOC 671\n"); }
void unalloc672() { printf("UNALLOC 672\n"); }
void unalloc673() { printf("UNALLOC 673\n"); }
void unalloc674() { printf("UNALLOC 674\n"); }
void unalloc675() { printf("UNALLOC 675\n"); }
void unalloc676() { printf("UNALLOC 676\n"); }
void unalloc677() { printf("UNALLOC 677\n"); }
void unalloc678() { printf("UNALLOC 678\n"); }
void unalloc679() { printf("UNALLOC 679\n"); }
void unalloc680() { printf("UNALLOC 680\n"); }
void unalloc681() { printf("UNALLOC 681\n"); }
void unalloc682() { printf("UNALLOC 682\n"); }
void unalloc683() { printf("UNALLOC 683\n"); }
void unalloc684() { printf("UNALLOC 684\n"); }
void unalloc685() { printf("UNALLOC 685\n"); }
void unalloc686() { printf("UNALLOC 686\n"); }
void unalloc687() { printf("UNALLOC 687\n"); }
void unalloc688() { printf("UNALLOC 688\n"); }
void unalloc689() { printf("UNALLOC 689\n"); }
void unalloc690() { printf("UNALLOC 690\n"); }
void unalloc691() { printf("UNALLOC 691\n"); }
void unalloc692() { printf("UNALLOC 692\n"); }
void unalloc693() { printf("UNALLOC 693\n"); }
void unalloc694() { printf("UNALLOC 694\n"); }
void unalloc695() { printf("UNALLOC 695\n"); }
void unalloc696() { printf("UNALLOC 696\n"); }
void unalloc697() { printf("UNALLOC 697\n"); }
void unalloc698() { printf("UNALLOC 698\n"); }
void unalloc699() { printf("UNALLOC 699\n"); }
void unalloc700() { printf("UNALLOC 700\n"); }

VoidFunc sym[700] = {
unalloc1,
unalloc2,
unalloc3,
unalloc4,
unalloc5,
unalloc6,
unalloc7,
unalloc8,
unalloc9,
unalloc10,
unalloc11,
unalloc12,
unalloc13,
unalloc14,
unalloc15,
unalloc16,
unalloc17,
unalloc18,
unalloc19,
unalloc20,
unalloc21,
unalloc22,
unalloc23,
unalloc24,
unalloc25,
unalloc26,
unalloc27,
unalloc28,
unalloc29,
unalloc30,
unalloc31,
(VoidFunc)app_event_loop,
unalloc33,
unalloc34,
(VoidFunc)app_log_trace,
unalloc36,
unalloc37,
unalloc38,
unalloc39,
unalloc40,
unalloc41,
unalloc42,
unalloc43,
unalloc44,
unalloc45,
unalloc46,
unalloc47,
unalloc48,
unalloc49,
unalloc50,
unalloc51,
unalloc52,
unalloc53,
unalloc54,                // battery_state_service_peek,
unalloc55,                // battery_state_service_subscribe,
unalloc56,                // battery_state_service_unsubscribe,
(VoidFunc)bitmap_layer_create,
(VoidFunc)bitmap_layer_destroy,
(VoidFunc)bitmap_layer_get_layer,
unalloc60,
unalloc61,
(VoidFunc)bitmap_layer_set_bitmap,
unalloc63,
unalloc64,
unalloc65,
unalloc66,
unalloc67,
unalloc68,
unalloc69,
(VoidFunc)pbl_clock_is_24h_style,                   //clock_is_24h_style
(VoidFunc)cos_lookup,
unalloc72,
unalloc73,
unalloc74,
unalloc75,
unalloc76,
unalloc77,
unalloc78,
unalloc79,
unalloc80,
unalloc81,
unalloc82,
unalloc83,
unalloc84,
unalloc85,
unalloc86,
unalloc87,
unalloc88,
unalloc89,
unalloc90,
unalloc91,
unalloc92,
unalloc93,
unalloc94,
unalloc95,
unalloc96,
(VoidFunc)fonts_get_system_font,
(VoidFunc)fonts_load_custom_font_proxy,                                              //custom font
unalloc99,
(VoidFunc)app_free,
(VoidFunc)gbitmap_create_as_sub_bitmap,
(VoidFunc)gbitmap_create_with_data,
(VoidFunc)gbitmap_create_with_resource_proxy,
(VoidFunc)gbitmap_destroy,
unalloc105,
(VoidFunc)n_gpath_create,
(VoidFunc)n_gpath_destroy,
unalloc108,
(VoidFunc)gpath_draw_app,
(VoidFunc)gpath_move_to_app,
(VoidFunc)gpath_rotate_to_app,
unalloc112, //(VoidFunc)gpoint_equal,
unalloc113,
unalloc114,
unalloc115,
unalloc116,
unalloc117, // (VoidFunc)graphics_draw_bitmap_in_rect_app,
(VoidFunc)graphics_draw_circle_app,
(VoidFunc)graphics_draw_line_app,
(VoidFunc)graphics_draw_pixel_app,
(VoidFunc)graphics_draw_rect_app,
unalloc122,
(VoidFunc)graphics_fill_circle_app,
(VoidFunc)graphics_fill_rect_app,
unalloc125,
unalloc126,
unalloc127,
(VoidFunc)n_graphics_center_point_rect,
unalloc129,
unalloc130,
unalloc131,
(VoidFunc)grect_equal,
unalloc133,
unalloc134,
unalloc135,
unalloc136,
unalloc137,
unalloc138,
(VoidFunc)layer_add_child,
(VoidFunc)layer_create,
unalloc141,
unalloc142,
(VoidFunc)layer_get_bounds,
unalloc144,
unalloc145,
(VoidFunc)layer_get_frame,
unalloc147,
unalloc148,
unalloc149,
unalloc150,
(void*)layer_mark_dirty,
unalloc152,
unalloc153,
unalloc154,
unalloc155,
(VoidFunc)layer_set_frame,
(VoidFunc)layer_set_hidden,
(VoidFunc)layer_set_update_proc,
unalloc159,
unalloc160,
(VoidFunc)rebble_time_get_tm,
unalloc162,
unalloc163,
unalloc164,
(VoidFunc)memset,
unalloc166,
unalloc167,
unalloc168,
unalloc169,
unalloc170,
unalloc171,
unalloc172,
unalloc173,
unalloc174,
unalloc175,
unalloc176,
unalloc177,
unalloc178,
unalloc179,
unalloc180,
unalloc181,
unalloc182,
unalloc183,
unalloc184,
unalloc185,
unalloc186,
unalloc187,
unalloc188,
(VoidFunc)persist_exists,
unalloc190,
unalloc191,
unalloc192,
unalloc193,
unalloc194,
unalloc195,
unalloc196,
unalloc197,
unalloc198,
unalloc199,
unalloc200,
unalloc201,
unalloc202,
unalloc203,
unalloc204,
unalloc205,
(VoidFunc)rand,
(VoidFunc)resource_get_handle_proxy,
unalloc208,
unalloc209,
unalloc210,
unalloc211,
unalloc212,
unalloc213,
unalloc214,
unalloc215,
unalloc216,
unalloc217,
unalloc218,
unalloc219,
unalloc220,
unalloc221,
unalloc222,
unalloc223,
unalloc224,
unalloc225,
unalloc226,
unalloc227,
unalloc228,
unalloc229,
unalloc230,
unalloc231,
unalloc232,
unalloc233,
unalloc234,
unalloc235,
unalloc236,
unalloc237,
unalloc238,
(VoidFunc)sin_lookup,
(VoidFunc)snprintf,                        // snprintf,
(VoidFunc)srand,                        // srand,
(VoidFunc)strcat,                        // strcat,
(VoidFunc)strcmp,                        // strcmp,
(VoidFunc)strcpy,                        // strcpy,
unalloc245,    //strftime
unalloc246,
unalloc247,
unalloc248,
unalloc249,
unalloc250,
unalloc251,
unalloc252,
unalloc253,
unalloc254,
unalloc255,
unalloc256,
unalloc257,
unalloc258,
unalloc259,
unalloc260,
unalloc261,
unalloc262,
(VoidFunc)tick_timer_service_subscribe,
unalloc264,
(VoidFunc)pbl_time_ms_deprecated,
unalloc266,
unalloc267,
unalloc268,
unalloc269,
unalloc270,
unalloc271,
(VoidFunc)window_create,
(VoidFunc)window_destroy,
unalloc274,
unalloc275,
(VoidFunc)window_get_root_layer,
unalloc277,
unalloc278,
unalloc279,
unalloc280,
unalloc281,
unalloc282,
(VoidFunc)window_set_window_handlers,
unalloc284,
unalloc285,
unalloc286,
unalloc287,
(VoidFunc)window_stack_push,
unalloc289,
unalloc290,
unalloc291,
unalloc292,
unalloc293,
unalloc294,
unalloc295,
unalloc296,
unalloc297,
unalloc298,
unalloc299,
unalloc300,
unalloc301,
unalloc302,
unalloc303,
(VoidFunc)window_long_click_subscribe,             // UNVERIFIED
(VoidFunc)window_multi_click_subscribe,            // UNVERIFIED
(VoidFunc)window_raw_click_subscribe,              // UNVERIFIED
(VoidFunc)window_set_click_context,                // UNVERIFIED
(VoidFunc)window_single_click_subscribe,           // UNVERIFIED
(VoidFunc)window_single_repeating_click_subscribe, // UNVERIFIED
(VoidFunc)graphics_draw_text_app,
unalloc311,
unalloc312,
unalloc313,
unalloc314,
unalloc315,
unalloc316,
unalloc317,
unalloc318,
(VoidFunc)bitmap_layer_get_bitmap,
unalloc320,
unalloc321,
unalloc322,
unalloc323,
unalloc324,
unalloc325,
unalloc326,
unalloc327,
unalloc328,
unalloc329,
unalloc330,
unalloc331,
unalloc332,
unalloc333,
unalloc334,
unalloc335,
unalloc336,
unalloc337,
unalloc338,
unalloc339,
unalloc340,
unalloc341,
unalloc342,
unalloc343,
(VoidFunc)gpath_fill_app, //unalloc344,
unalloc345,
unalloc346,
unalloc347,
unalloc348,
unalloc349,
unalloc350,
unalloc351,
unalloc352,
unalloc353,
unalloc354,
unalloc355,
unalloc356,
unalloc357,
unalloc358,
unalloc359,
unalloc360,
unalloc361,
unalloc362,
unalloc363,
unalloc364,
unalloc365,
unalloc366,
unalloc367,
unalloc368,
unalloc369,
unalloc370,
unalloc371,
(VoidFunc)graphics_context_set_fill_color,
(VoidFunc)graphics_context_set_stroke_color,
(VoidFunc)graphics_context_set_text_color,
unalloc375,
unalloc376,
unalloc377,
unalloc378,
unalloc379,
unalloc380,
unalloc381, // animation_destroy
unalloc382,
unalloc383,
unalloc384,
unalloc385, // animation_schedule
unalloc386,
unalloc387,
unalloc388,
unalloc389, // animation_set_handlers
unalloc390,
unalloc391, // animation_unschedule
unalloc392,
unalloc393,
unalloc394,
unalloc395,
unalloc396,
unalloc397,
unalloc398,
unalloc399,
unalloc400,
unalloc401,
unalloc402,
unalloc403,
unalloc404,
unalloc405,
unalloc406,
unalloc407,
(VoidFunc)gbitmap_get_bounds,
(VoidFunc)gbitmap_get_bytes_per_row,
(VoidFunc)gbitmap_get_data,
unalloc411,
unalloc412,
unalloc413,
unalloc414,
unalloc415,
unalloc416,
unalloc417,
unalloc418,
unalloc419,
unalloc420,
unalloc421,
unalloc422,
unalloc423,
unalloc424,
unalloc425,
unalloc426,
unalloc427,
unalloc428,
unalloc429,
unalloc430,
unalloc431,
unalloc432,
unalloc433,
unalloc434,
unalloc435,
unalloc436,
unalloc437,
unalloc438,
unalloc439,
unalloc440,
unalloc441,
unalloc442,
unalloc443,
unalloc444,
unalloc445,
(VoidFunc)graphics_context_set_stroke_width,
unalloc447,
unalloc448,
unalloc449,
unalloc450,
unalloc451,
unalloc452,
unalloc453,
unalloc454,
unalloc455,
unalloc456,
unalloc457,
unalloc458,
unalloc459,
unalloc460,
unalloc461,
unalloc462,
(VoidFunc)text_layer_create, //unalloc463,
(VoidFunc)text_layer_destroy,                        // text_layer_destroy,
(VoidFunc)text_layer_get_content_size,                        // text_layer_get_content_size,
(VoidFunc)text_layer_get_layer,                        // text_layer_get_layer,
(VoidFunc)text_layer_get_text,                        // text_layer_get_text,
(VoidFunc)text_layer_set_background_color,                        // text_layer_set_background_color,
(VoidFunc)text_layer_set_font,                        // text_layer_set_font,
(VoidFunc)text_layer_set_overflow_mode,                        // text_layer_set_overflow_mode,
(VoidFunc)text_layer_set_size,                        // text_layer_set_size,
(VoidFunc)text_layer_set_text,                        // text_layer_set_text,
(VoidFunc)text_layer_set_text_alignment,                        // text_layer_set_text_alignment,
(VoidFunc)text_layer_set_text_color,
(VoidFunc)unalloc475,
(VoidFunc)n_gdraw_command_draw,                        // gdraw_command_draw,unalloc476,
(VoidFunc)n_gdraw_command_frame_draw,                        // gdraw_command_frame_draw,unalloc477,
(VoidFunc)n_gdraw_command_frame_get_duration,                        // gdraw_command_frame_get_duration,unalloc478,
(VoidFunc)n_gdraw_command_frame_set_duration,                        // gdraw_command_frame_set_duration,unalloc479,
(VoidFunc)n_gdraw_command_get_fill_color,                        // gdraw_command_get_fill_color,unalloc480,
(VoidFunc)n_gdraw_command_get_hidden,                        // gdraw_command_get_hidden,unalloc481,
(VoidFunc)n_gdraw_command_get_num_points,                        // gdraw_command_get_num_points,unalloc482,
(VoidFunc)n_gdraw_command_get_path_open,                        // gdraw_command_get_path_open,unalloc483,
(VoidFunc)n_gdraw_command_get_point,                        // gdraw_command_get_point,unalloc484,
(VoidFunc)n_gdraw_command_get_radius,                        // gdraw_command_get_radius,unalloc485,
(VoidFunc)n_gdraw_command_get_stroke_color,                        // gdraw_command_get_stroke_color,unalloc486,
(VoidFunc)n_gdraw_command_get_stroke_width,                        // gdraw_command_get_stroke_width,unalloc487,
(VoidFunc)n_gdraw_command_get_type,                        // gdraw_command_get_type,unalloc488,
(VoidFunc)n_gdraw_command_image_clone,                        // gdraw_command_image_clone,unalloc489,
(VoidFunc)n_gdraw_command_image_create_with_resource,                        // gdraw_command_image_create_with_resource,unalloc490,
(VoidFunc)n_gdraw_command_image_destroy,                        // gdraw_command_image_destroy,unalloc491,
(VoidFunc)n_gdraw_command_image_draw,                        // gdraw_command_image_draw,unalloc492,
(VoidFunc)n_gdraw_command_image_get_bounds_size,                        // gdraw_command_image_get_bounds_size,unalloc493,
(VoidFunc)n_gdraw_command_image_get_command_list,                        // gdraw_command_image_get_command_list,unalloc494,
(VoidFunc)n_gdraw_command_image_set_bounds_size,                        // gdraw_command_image_set_bounds_size,unalloc495,
(VoidFunc)n_gdraw_command_list_draw,                        // gdraw_command_list_draw,unalloc496,
(VoidFunc)n_gdraw_command_list_get_command,                        // gdraw_command_list_get_command,unalloc497,
(VoidFunc)n_gdraw_command_list_get_num_commands,                        // gdraw_command_list_get_num_commands,unalloc498,
(VoidFunc)n_gdraw_command_list_iterate,                        // gdraw_command_list_iterate,unalloc499,
(VoidFunc)n_gdraw_command_sequence_clone,                        // gdraw_command_sequence_clone,unalloc500,
(VoidFunc)n_gdraw_command_sequence_create_with_resource,                        // gdraw_command_sequence_create_with_resource,unalloc501,
(VoidFunc)n_gdraw_command_sequence_destroy,                        // gdraw_command_sequence_destroy,unalloc502,
(VoidFunc)n_gdraw_command_sequence_get_bounds_size,                        // gdraw_command_sequence_get_bounds_size,unalloc503,
(VoidFunc)n_gdraw_command_sequence_get_frame_by_elapsed,                        // gdraw_command_sequence_get_frame_by_elapsed,unalloc504,
(VoidFunc)n_gdraw_command_sequence_get_frame_by_index,                        // gdraw_command_sequence_get_frame_by_index,unalloc505,
(VoidFunc)n_gdraw_command_sequence_get_num_frames,                        // gdraw_command_sequence_get_num_frames,unalloc506,
(VoidFunc)n_gdraw_command_sequence_get_play_count,                        // gdraw_command_sequence_get_play_count,unalloc507,
(VoidFunc)n_gdraw_command_sequence_get_total_duration,                        // gdraw_command_sequence_get_total_duration,unalloc508,
(VoidFunc)n_gdraw_command_sequence_set_bounds_size,                        // gdraw_command_sequence_set_bounds_size,unalloc509,
(VoidFunc)n_gdraw_command_sequence_set_play_count,                        // gdraw_command_sequence_set_play_count,unalloc510,
(VoidFunc)n_gdraw_command_set_fill_color,                        // gdraw_command_set_fill_color,unalloc511,
(VoidFunc)n_gdraw_command_set_hidden,                        // gdraw_command_set_hidden,unalloc512,
(VoidFunc)n_gdraw_command_set_path_open,                        // gdraw_command_set_path_open,unalloc513,
(VoidFunc)n_gdraw_command_set_point,                        // gdraw_command_set_point,unalloc514,
(VoidFunc)n_gdraw_command_set_radius,                        // gdraw_command_set_radius,unalloc515,
(VoidFunc)n_gdraw_command_set_stroke_color,                        // gdraw_command_set_stroke_color,unalloc516,
(VoidFunc)n_gdraw_command_set_stroke_width,                        // gdraw_command_set_stroke_width,unalloc517,
unalloc518,
(VoidFunc)gpath_draw_app,
unalloc520,
unalloc521,
unalloc522,
unalloc523,
unalloc524,
unalloc525,
unalloc526,
unalloc527,
unalloc528,
unalloc529,
unalloc530,
unalloc531,
unalloc532,
unalloc533,
unalloc534,
unalloc535,
unalloc536,
unalloc537,
unalloc538,
unalloc539,
unalloc540,
unalloc541,
unalloc542,
unalloc543,
unalloc544,
unalloc545,
unalloc546,
unalloc547,
unalloc548,
unalloc549,
unalloc550,
unalloc551,
unalloc552,
unalloc553,
unalloc554,
unalloc555,
unalloc556,
unalloc557,
unalloc558,
unalloc559,
unalloc560,
unalloc561,
unalloc562,
unalloc563,
unalloc564,
unalloc565,
unalloc566,
unalloc567,
unalloc568,
unalloc569,
unalloc570,
unalloc571,
unalloc572,
unalloc573,
unalloc574,
unalloc575,
unalloc576,
unalloc577,
unalloc578,
unalloc579,
unalloc580,
unalloc581,
unalloc582,
unalloc583,
unalloc584,
unalloc585,
unalloc586,
unalloc587,
unalloc588,
unalloc589,
unalloc590,
unalloc591,
unalloc592,
unalloc593,
unalloc594,
unalloc595,
unalloc596,
unalloc597,
unalloc598,
unalloc599,
unalloc600,
unalloc601,
unalloc602,
unalloc603,
unalloc604,
unalloc605,
unalloc606,
unalloc607,
unalloc608,
unalloc609,
unalloc610,
unalloc611,
unalloc612,
unalloc613,
unalloc614,
unalloc615,
unalloc616,
unalloc617,
unalloc618,
unalloc619,
unalloc620,
unalloc621,
unalloc622,
(VoidFunc)layer_get_unobstructed_bounds,
unalloc624,
unalloc625,
unalloc626,
unalloc627,
unalloc628,
unalloc629,
unalloc630,
unalloc631,
unalloc632,
unalloc633,
unalloc634,
unalloc635,
unalloc636,
unalloc637,
unalloc638,
unalloc639,
unalloc640,
unalloc641,
unalloc642,
unalloc643,
unalloc644,
unalloc645,
unalloc646,
unalloc647,
unalloc648,
unalloc649,
unalloc650,
unalloc651,
unalloc652,
unalloc653,
unalloc654,
unalloc655,
unalloc656,
unalloc657,
unalloc658,
unalloc659,
unalloc660,
unalloc661,
unalloc662,
unalloc663,
unalloc664,
unalloc665,
unalloc666,
unalloc667,
unalloc668,
unalloc669,
unalloc670,
unalloc671,
unalloc672,
unalloc673,
unalloc674,
unalloc675,
unalloc676,
unalloc677,
unalloc678,
unalloc679,
unalloc680,
unalloc681,
unalloc682,
unalloc683,
unalloc684,
unalloc685,
unalloc686,
unalloc687,
unalloc688,
unalloc689,
unalloc690,
unalloc691,
unalloc692,
unalloc693,
unalloc694,
unalloc695,
unalloc696,
unalloc697,
unalloc698,
unalloc699,
unalloc700,
};
