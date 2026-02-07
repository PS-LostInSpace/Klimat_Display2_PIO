#include "PageManager.h"
#include "page1.h"

// -----------------------------------------------------------------------------
// Internal state
// -----------------------------------------------------------------------------

enum PageId {
    PAGE_1 = 0,
};

static PageId g_current_page = PAGE_1;
static lv_obj_t* g_root = nullptr;


// -----------------------------------------------------------------------------
// Begin
// -----------------------------------------------------------------------------

void pagemgr_begin(lv_obj_t* root) {
    g_root = root;

    switch (g_current_page) {
        case PAGE_1:
            page1_build(g_root);
            break;
    }
}


// -----------------------------------------------------------------------------
// Update (called from LVGL loop)
// -----------------------------------------------------------------------------

void pagemgr_update() {
    switch (g_current_page) {
        case PAGE_1:
            page1_update();
            break;
    }
}
