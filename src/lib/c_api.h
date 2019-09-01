#ifndef SYNERGY_C_API
#define SYNERGY_C_API

#ifdef __cplusplus
extern "C" {
#endif

struct synergy_c_ops {
    void (*bgthread_cb)(void);
    void (*change_focus_cb)(bool enter);
    void (*keypress_cb)(int key, int mask, int button);
    void (*exit_cb)(void);
};

void syn_reinit(const struct synergy_c_ops *ops);

void syn_press_key(int key, int mask, int button);
void syn_unpress_key(int key, int mask, int button);
void syn_enter_screen(void);
void syn_leave_screen(void);
void syn_get_mouse(int *x, int *y);
void syn_click_mouse(int button);
void syn_unclick_mouse(int button);
void syn_mouse_move(int x, int y);

#ifdef __cplusplus
}
#endif

#endif
