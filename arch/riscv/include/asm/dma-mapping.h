/* SPDX-License-Identifier: GPL-2.0 */

extern const struct dma_map_ops aw_ansc_dma_ops;

static inline const struct dma_map_ops *get_arch_dma_ops(struct bus_type *bus)
{
	if (IS_ENABLED(CONFIG_AW_ANSC_DMA_OPS))
		return &aw_ansc_dma_ops;
	return NULL;
}