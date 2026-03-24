# Backlog

## Patch
> v1.1.2 - critical bug fixes

-[ ] Fix missing closing parenthesis in FBGL_F32RGBA_TO_U32 macro — currently fails to compile _bug_
-[ ] Fix fbgl_draw_line termination condition — breaks for lines with negative direction (right-to-left or bottom-to-top) _bug_
-[ ] Fix i_fbgl_sqrt_int — currently computes x*x (square) not sqrt, corrupting filled circle rendering _bug_
-[ ] Implement missing fbgl_clear — declared in header but has no definition; rename or alias from fbgl_set_bg _bug_

## Minor
> v1.2.0 - input and window system

-[ ] Fix fbgl_is_key_pressed consuming the keypress via fbgl_get_key — peek without consuming _bug_
-[ ] Add mouse input support — position, left/right/middle button state _feat_
-[ ] Implement fbgl_window_t API — fbgl_window_create, fbgl_window_destroy, clipped drawing within window bounds _feat_
-[ ] Add line thickness parameter to fbgl_draw_line _feat_
-[ ] Add fbgl_draw_triangle_outline and fbgl_draw_triangle_filled _feat_
-[ ] Add fbgl_draw_ellipse_outline and fbgl_draw_ellipse_filled _feat_
-[ ] Add fbgl_draw_arc primitive _feat_

## Minor
> v1.3.0 - rendering quality

-[ ] Add double buffering — off-screen back buffer with explicit fbgl_swap_buffers to eliminate tearing _feat_
-[ ] Add alpha blending / compositing — partial transparency instead of binary draw-or-skip _feat_
-[ ] Add clipping region / scissor rect support _feat_
-[ ] Add scaled texture rendering — fbgl_draw_texture_scaled(fb, tex, x, y, w, h) _feat_
-[ ] Add PSF2 font support alongside existing PSF1feat
-[ ] Add polygon drawing — fbgl_draw_polygon_outline and fbgl_draw_polygon_filled with vertex array _feat_

## Minor
> v1.4.0 - performance & dx

-[ ] Add thread safety — mutex guards on g_keyboard_state and previous_frame_time globals _dx_
-[ ] Batch pixel writes with memset / SIMD where applicable for fbgl_set_bg and filled shapes _perf_
-[ ] Add dirty-rect tracking — only flush changed regions to framebuffer _perf_
-[ ] Add FBGL_NO_MATH compile flag to explicitly opt out of math.h dependency _dx_
-[ ] Add comprehensive error code enum — replace magic -1 returns with named codes _dx_
-[ ] Add debug logging mode with FBGL_DEBUG flag covering all drawing functions, not just fbgl_set_bg _dx_

## Major
> v2.0.0 - multi-display & extended formats

-[ ] Multi-display support — manage multiple /dev/fbN devices simultaneously _feat_
-[ ] PNG texture loader alongside existing TGA — reduces external tooling dependency _feat_
-[ ] Sprite sheet / atlas support with UV rect selection _feat_
-[ ] TTF/BDF font rasterizer — remove hard dependency on PSF bitmap fonts _feat_
-[ ] Break fbgl_window_t into full sub-surface compositor with z-ordering _feat_
