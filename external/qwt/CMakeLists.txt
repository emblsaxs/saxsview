project (QWT CXX)

qt4_wrap_cpp (MOC qwt_abstract_slider.h qwt_analog_clock.h qwt_compass.h
                  qwt_counter.h qwt_dial.h qwt_dyngrid_layout.h qwt_knob.h
                  qwt_legend.h qwt_magnifier.h
                  qwt_panner.h qwt_picker.h qwt_plot.h qwt_plot_canvas.h
                  qwt_plot_magnifier.h qwt_plot_panner.h qwt_plot_picker.h
                  qwt_plot_zoomer.h qwt_scale_widget.h qwt_slider.h
                  qwt_text_label.h qwt_thermo.h qwt_wheel.h
                  qwt_plot_renderer.h qwt_sampling_thread.h
                  qwt_abstract_legend.h qwt_legend_label.h qwt_series_store.h)

set (HEADERS qwt.h qwt_dial_needle.h qwt_plot_magnifier.h
             qwt_scale_div.h qwt_abstract_scale.h 
             qwt_painter.h qwt_plot_marker.h qwt_scale_draw.h
             qwt_abstract_scale_draw.h qwt_double_range.h qwt_panner.h
             qwt_plot_panner.h qwt_scale_engine.h qwt_abstract_slider.h
             qwt_picker.h qwt_plot_picker.h qwt_scale_map.h
             qwt_analog_clock.h qwt_dyngrid_layout.h qwt_picker_machine.h
             qwt_scale_widget.h 
             qwt_event_pattern.h qwt_plot.h qwt_plot_rasteritem.h
             qwt_series_data.h qwt_arrow_button.h qwt_global.h
             qwt_plot_canvas.h qwt_plot_rescaler.h qwt_slider.h qwt_clipper.h
             qwt_interval_symbol.h qwt_plot_curve.h qwt_plot_scaleitem.h
             qwt_spline.h qwt_color_map.h qwt_knob.h qwt_plot_dict.h
             qwt_plot_seriesitem.h qwt_symbol.h qwt_column_symbol.h
             qwt_plot_directpainter.h
             qwt_plot_spectrogram.h qwt_text.h qwt_compass.h qwt_legend.h
             qwt_plot_grid.h qwt_plot_svgitem.h qwt_text_engine.h
             qwt_compass_rose.h qwt_plot_histogram.h
             qwt_plot_zoomer.h qwt_text_label.h qwt_counter.h
             qwt_plot_intervalcurve.h
             qwt_thermo.h qwt_curve_fitter.h qwt_magnifier.h qwt_plot_item.h
             qwt_raster_data.h qwt_dial.h qwt_math.h
             qwt_plot_layout.h qwt_round_scale_draw.h qwt_wheel.h
             qwt_plot_renderer.h qwt_sampling_thread.h 
             qwt_plot_spectrocurve.h qwt_system_clock.h qwt_compat.h
             qwt_abstract_legend.h qwt_graphic.h qwt_interval.h qwt_legend_data.h
             qwt_legend_label.h qwt_matrix_raster_data.h qwt_null_paintdevice.h
             qwt_painter_command.h qwt_pixel_matrix.h qwt_plot_barchart.h qwt_plot_baritem.h
             qwt_plot_legenditem.h qwt_plot_multi_barchart.h qwt_plot_tradingcurve.h
             qwt_point_3d.h qwt_point_data.h qwt_point_mapper.h qwt_point_polar.h
             qwt_samples.h qwt_series_store.h)


set (SOURCES qwt_abstract_scale.cpp qwt_double_range.cpp
             qwt_picker_machine.cpp qwt_plot_picker.cpp qwt_scale_engine.cpp
             qwt_abstract_scale_draw.cpp qwt_plot.cpp
             qwt_scale_map.cpp qwt_abstract_slider.cpp
             qwt_dyngrid_layout.cpp qwt_plot_axis.cpp 
             qwt_scale_widget.cpp qwt_analog_clock.cpp qwt_event_pattern.cpp
             qwt_plot_canvas.cpp qwt_plot_rasteritem.cpp qwt_series_data.cpp
             qwt_arrow_button.cpp qwt_interval_symbol.cpp qwt_plot_curve.cpp
             qwt_plot_rescaler.cpp qwt_slider.cpp qwt_clipper.cpp qwt_knob.cpp
             qwt_plot_dict.cpp qwt_plot_scaleitem.cpp qwt_spline.cpp
             qwt_color_map.cpp 
             qwt_plot_directpainter.cpp qwt_plot_seriesitem.cpp qwt_symbol.cpp
             qwt_column_symbol.cpp qwt_legend.cpp qwt_plot_grid.cpp
             qwt_plot_spectrogram.cpp qwt_text.cpp qwt_compass.cpp
             qwt_text_engine.cpp qwt_compass_rose.cpp qwt_magnifier.cpp
             qwt_plot_histogram.cpp qwt_plot_svgitem.cpp
             qwt_plot_intervalcurve.cpp qwt_plot_xml.cpp qwt_text_label.cpp
             qwt_counter.cpp qwt_math.cpp qwt_plot_item.cpp qwt_plot_zoomer.cpp
             qwt_thermo.cpp qwt_curve_fitter.cpp
             qwt_plot_layout.cpp qwt_raster_data.cpp qwt_wheel.cpp qwt_dial.cpp
             qwt_painter.cpp qwt_plot_magnifier.cpp qwt_round_scale_draw.cpp
             qwt_dial_needle.cpp qwt_panner.cpp qwt_plot_marker.cpp
             qwt_scale_div.cpp qwt_picker.cpp
             qwt_plot_panner.cpp qwt_scale_draw.cpp qwt_plot_renderer.cpp
             qwt_system_clock.cpp qwt_sampling_thread.cpp
             qwt_plot_spectrocurve.cpp qwt_abstract_legend.cpp
             qwt_graphic.cpp qwt_interval.cpp qwt_legend_data.cpp
             qwt_legend_label.cpp qwt_matrix_raster_data.cpp qwt_null_paintdevice.cpp
             qwt_painter_command.cpp qwt_pixel_matrix.cpp qwt_plot_barchart.cpp
             qwt_plot_baritem.cpp qwt_plot_legenditem.cpp qwt_plot_multi_barchart.cpp
             qwt_plot_tradingcurve.cpp qwt_point_3d.cpp qwt_point_data.cpp
             qwt_point_mapper.cpp qwt_point_polar.cpp)

saxsview_add_library(qwt SOURCES ${HEADERS} ${SOURCES} ${MOC}
                         LIBRARIES ${QT_LIBRARIES}
                         VERSION 6.1.0
                         SHARED)

saxsview_install(TARGETS qwt)

