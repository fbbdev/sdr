/*
 * Nuklear - 1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2016 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_GLFW_GL3_H_
#define NK_GLFW_GL3_H_

#include <GLFW/glfw3.h>

#ifdef __cplusplus
extern "C" {
#endif

enum nk_glfw_init_state{
    NK_GLFW3_DEFAULT=0,
    NK_GLFW3_INSTALL_CALLBACKS
};

struct nk_glfw3_context {
/* public */
    struct nk_context* ctx;
    void* userdata;

/* private */
    struct nk_glfw* glfw;
};

NK_API struct nk_glfw3_context* nk_glfw3_init(GLFWwindow *win, enum nk_glfw_init_state);
NK_API void                     nk_glfw3_shutdown(struct nk_glfw3_context* gctx);
NK_API void                     nk_glfw3_font_stash_begin(struct nk_glfw3_context* gctx, struct nk_font_atlas **atlas);
NK_API void                     nk_glfw3_font_stash_end(struct nk_glfw3_context* gctx);
NK_API void                     nk_glfw3_new_frame(struct nk_glfw3_context* gctx);
NK_API void                     nk_glfw3_render(struct nk_glfw3_context* gctx, enum nk_anti_aliasing, int max_vertex_buffer, int max_element_buffer);

NK_API void                     nk_glfw3_device_destroy(struct nk_glfw3_context* gctx);
NK_API void                     nk_glfw3_device_create(struct nk_glfw3_context* gctx);

NK_API void                     nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint);
NK_API void                     nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff);
NK_API void                     nk_glfw3_mouse_button_callback(GLFWwindow *win, int button, int action, int mods);

#ifdef __cplusplus
}
#endif

#endif
