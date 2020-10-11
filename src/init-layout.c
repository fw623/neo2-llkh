#define UNICODE

#include "init-layout.h"

void mapLevels_2_5_6(TCHAR * mappingTableOutput, TCHAR * newChars) {
	TCHAR * l1_lowercase = L"abcdefghijklmnopqrstuvwxyzäöüß.,";

	TCHAR *ptr;
	for (int i = 0; i < LEN; i++) {
		ptr = wcschr(l1_lowercase, mappingTableLevel1[i]);
		if (ptr != NULL && ptr < &l1_lowercase[32]) {
			//printf("i = %d: mappingTableLevel1[i] = %c; ptr = %d; ptr = %s; index = %d\n", i, mappingTableLevel1[i], ptr, ptr, ptr-l1_lowercase+1);
			mappingTableOutput[i] = newChars[ptr-l1_lowercase];
		}
	}
}

void initLevel4SpecialCases() {
	for (int i = 0; i < LEN; i++)
		mappingTableLevel4Special[i] = 0;

	mappingTableLevel4Special[16] = VK_PRIOR;
	if (strcmp(layout, "kou-fw623") == 0) {
		mappingTableLevel4Special[18] = VK_UP;
		mappingTableLevel4Special[20] = VK_NEXT;
		mappingTableLevel4Special[43] = VK_RETURN;
	} else if (strcmp(layout, "kou") == 0 || strcmp(layout, "vou") == 0) {
		mappingTableLevel4Special[17] = VK_NEXT;
		mappingTableLevel4Special[18] = VK_UP;
		mappingTableLevel4Special[19] = VK_BACK;
		mappingTableLevel4Special[20] = VK_DELETE;
	} else {
		mappingTableLevel4Special[17] = VK_BACK;
		mappingTableLevel4Special[18] = VK_UP;
		mappingTableLevel4Special[19] = VK_DELETE;
		mappingTableLevel4Special[20] = VK_NEXT;
	}
	mappingTableLevel4Special[30] = VK_HOME;
	mappingTableLevel4Special[31] = VK_LEFT;
	mappingTableLevel4Special[32] = VK_DOWN;
	mappingTableLevel4Special[33] = VK_RIGHT;
	mappingTableLevel4Special[34] = VK_END;
	if (strcmp(layout, "kou") == 0 || strcmp(layout, "vou") == 0) {
		mappingTableLevel4Special[44] = VK_INSERT;
		mappingTableLevel4Special[45] = VK_TAB;
		mappingTableLevel4Special[46] = VK_RETURN;
		mappingTableLevel4Special[47] = VK_ESCAPE;
	} else if (strcmp(layout, "kou-fw623") == 0) {
		mappingTableLevel4Special[44] = VK_SPACE;
		mappingTableLevel4Special[45] = VK_SPACE;
		mappingTableLevel4Special[46] = VK_RETURN;
		mappingTableLevel4Special[47] = VK_SPACE;
		mappingTableLevel4Special[49] = VK_BACK; // new special case
	} else {
		mappingTableLevel4Special[44] = VK_ESCAPE;
		mappingTableLevel4Special[45] = VK_TAB;
		mappingTableLevel4Special[46] = VK_INSERT;
		mappingTableLevel4Special[47] = VK_RETURN;
	}

	if (strcmp(layout, "kou-fw623") != 0)
		mappingTableLevel4Special[57] = '0';
}

void initLayout() {
	// initialize the mapping tables
	for (int i = 0; i < LEN; i++) {
		mappingTableLevel1[i] = 0;
		mappingTableLevel2[i] = 0;
		mappingTableLevel3[i] = 0;
		mappingTableLevel4[i] = 0;
		if (supportLevels5and6) {
			mappingTableLevel5[i] = 0;
			mappingTableLevel6[i] = 0;
		}
	}

	// same for all layouts
	wcscpy(mappingTableLevel1 +  2, L"1234567890-`");

	wcscpy(mappingTableLevel2 + 41, L"̌");  // key to the left of the "1" key
	wcscpy(mappingTableLevel2 + 2, strcmp(layout, "kou-fw623") == 0 ? L"°§ℓ»«¢€„“”–" : L"°§ℓ»«$€„“”—̧");

	wcscpy(mappingTableLevel3 + 41, L"^");
	wcscpy(mappingTableLevel3 + 2, strcmp(layout, "kou-fw623") == 0 ? L"¹²³›‹¥£‚‘’—" : L"¹²³›‹¢¥‚‘’—̊");
	wcscpy(mappingTableLevel3 + 16, L"…_[]^!<>=&ſ̷");
	wcscpy(mappingTableLevel3 + 30, L"\\/{}*?()-:@"); // TODO: fix right brace in vscode
	wcscpy(mappingTableLevel3 + 44, L"#$|~`+%\"';");

	wcscpy(mappingTableLevel4 + 41, L"̇");
	wcscpy(mappingTableLevel4 +  2, L"ªº№⋮·£¤0/*-¨");
	wcscpy(mappingTableLevel4 + 21, L"¡789+−˝");
	wcscpy(mappingTableLevel4 + 35, L"¿456,.");
	wcscpy(mappingTableLevel4 + 49, L":123;");

	// layout dependent
	if (strcmp(layout, "adnw") == 0) {
		wcscpy(mappingTableLevel1 + 16, L"kuü.ävgcljf´");
		wcscpy(mappingTableLevel1 + 30, L"hieaodtrnsß");
		wcscpy(mappingTableLevel1 + 44, L"xyö,qbpwmz");

	} else if (strcmp(layout, "adnwzjf") == 0) {
		wcscpy(mappingTableLevel1 + 16, L"kuü.ävgclßz´");
		wcscpy(mappingTableLevel1 + 30, L"hieaodtrnsf");
		wcscpy(mappingTableLevel1 + 44, L"xyö,qbpwmj");

	} else if (strcmp(layout, "bone") == 0) {
		wcscpy(mappingTableLevel1 + 16, L"jduaxphlmwß´");
		wcscpy(mappingTableLevel1 + 30, L"ctieobnrsgq");
		wcscpy(mappingTableLevel1 + 44, L"fvüäöyz,.k");

	} else if (strcmp(layout, "koy") == 0) {
		wcscpy(mappingTableLevel1 + 16, L"k.o,yvgclßz´");
		wcscpy(mappingTableLevel1 + 30, L"haeiudtrnsf");
		wcscpy(mappingTableLevel1 + 44, L"xqäüöbpwmj");

	} else if (strcmp(layout, "kou") == 0
				  || strcmp(layout, "kou-fw623") == 0
				  || strcmp(layout, "vou") == 0) {
		if (strcmp(layout, "kou") == 0 || strcmp(layout, "kou-fw623") == 0) {
			wcscpy(mappingTableLevel1 + 16, L"k.ouäqgclfj´");
			wcscpy(mappingTableLevel1 + 30, L"haeiybtrnsß");
			wcscpy(mappingTableLevel1 + 44, L"zx,üöpdwmv");
		} else {  // vou
			wcscpy(mappingTableLevel1 + 16, L"v.ouäqglhfj´");
			wcscpy(mappingTableLevel1 + 30, L"caeiybtrnsß");
			wcscpy(mappingTableLevel1 + 44, L"zx,üöpdwmk");
		}

		if (strcmp(layout, "kou-fw623") == 0) {
			wcscpy(mappingTableLevel3 + 16, L"@=:^• #<>~→̷");
			wcscpy(mappingTableLevel3 + 30, L"`'-/*&{()}|");
			wcscpy(mappingTableLevel3 + 44, L"\\\"_+ ;[]$%");

			wcscpy(mappingTableLevel4 + 4, L"    =/*-−¨");
			wcscpy(mappingTableLevel4 + 21, L";789+\t−˝");
			wcscpy(mappingTableLevel4 + 35, L":456.\n");
			wcscpy(mappingTableLevel4 + 48, L"·");
			wcscpy(mappingTableLevel4 + 50, L"123,");
		} else { // kou or vou
			wcscpy(mappingTableLevel3 + 16, L"@%{}^!<>=&€̷");
			wcscpy(mappingTableLevel3 + 30, L"|`()*?/:-_→");
			wcscpy(mappingTableLevel3 + 44, L"#[]~$+\"'\\;");

			wcscpy(mappingTableLevel4 + 4, L"✔✘·£¤0/*-¨");
			wcscpy(mappingTableLevel4 + 21, L":789+−˝");
			wcscpy(mappingTableLevel4 + 35, L"-456,;");
			wcscpy(mappingTableLevel4 + 49, L"_123.");
		}
	} else if (strcmp(layout, "qwertz") == 0) {
		wcscpy(mappingTableLevel1 + 12, L"ß");
		wcscpy(mappingTableLevel1 + 16, L"qwertzuiopü+");
		wcscpy(mappingTableLevel1 + 30, L"asdfghjklöä");
		wcscpy(mappingTableLevel1 + 44, L"yxcvbnm,.-");

	} else { // neo
		wcscpy(mappingTableLevel1 + 16, L"xvlcwkhgfqß´");
		wcscpy(mappingTableLevel1 + 30, L"uiaeosnrtdy");
		wcscpy(mappingTableLevel1 + 44, L"üöäpzbm,.j");
	}

	// same for all layouts
	wcscpy(mappingTableLevel1 + 27, L"´");
	wcscpy(mappingTableLevel2 + 27, L"~");

	// map letters of level 2
	TCHAR * charsLevel2;
	charsLevel2 = strcmp(layout, "kou-fw623") == 0 ? L"ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜẞ?!" : L"ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜẞ•–";
	mapLevels_2_5_6(mappingTableLevel2, charsLevel2);

	if (supportLevels5and6) {
		// map main block on levels 5 and 6
		TCHAR * charsLevel5 = L"αβχδεφγψιθκλμνοπϕρστuvωξυζηϵüςϑϱ";  // a-zäöüß.,
		mapLevels_2_5_6(mappingTableLevel5, charsLevel5);
		TCHAR * charsLevel6 = L"∀⇐ℂΔ∃ΦΓΨ∫Θ⨯Λ⇔ℕ∈ΠℚℝΣ∂⊂√ΩΞ∇ℤℵ∩∪∘↦⇒";  // a-zäöüß.,
		mapLevels_2_5_6(mappingTableLevel6, charsLevel6);

		// add number row and dead key in upper letter row
		wcscpy(mappingTableLevel5 + 41, L"̉");
		wcscpy(mappingTableLevel5 +  2, L"₁₂₃♂♀⚥ϰ⟨⟩₀?῾");
		wcscpy(mappingTableLevel5 + 27, L"᾿");
		mappingTableLevel5[57] = 0x00a0;  // space = no-break space
		wcscpy(mappingTableLevel6 + 41, L"̣");
		wcscpy(mappingTableLevel6 +  2, L"¬∨∧⊥∡∥→∞∝⌀?̄");
		wcscpy(mappingTableLevel6 + 27, L"˘");
		mappingTableLevel6[57] = 0x202f;  // space = narrow no-break space
	}

	mappingTableLevel2[8] = 0x20AC;  // €

	// level4 special cases
	initLevel4SpecialCases();
}
