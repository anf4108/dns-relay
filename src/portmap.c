#include <portmap.h>

PortMap * PortMap_create(int capacity)
{
	PortMap * pm = (PortMap *)malloc(sizeof(PortMap));
	if (pm == NULL) {
		log_error("[port map]：malloc Port Map");
		exit(-1);
	}
	pm->size = capacity;
	pm->contents = (struct content *)malloc(sizeof(struct content) * capacity);
	if (pm->contents == NULL) {
		log_error("[port map]：malloc struct content");
		free(pm);
		exit(-1);
	}
	pm->current_seq = 0;
	return pm;
}

bool PortMap_allocSeq(PortMap * pm, char ip[IP4SIZE], uint16_t local_seq, uint16_t * global_seq)
{
	// 内容放在下标为pm->currentSeq%size的地方
	strncpy(pm->contents[pm->current_seq % pm->size].ip, ip, IP4SIZE);
	pm->contents[pm->current_seq % pm->size].local_seq = local_seq;
	*global_seq = pm->current_seq;

	log_info("[port map]客户端ip = %u.%u.%u.%u, seq = %d, 分配得到Global Seq = %d ",
		(uint8_t)ip[0], (uint8_t)ip[1], (uint8_t)ip[2], (uint8_t)ip[3], local_seq, *global_seq);

	// 待分配序号++
	pm->current_seq++;
}
bool PortMap_querySeq(PortMap * pm, uint16_t global_seq, uint16_t * local_seq,  char ip[IP4SIZE])
{
	// 把下标为global_seq%size的地方的内容取出
	strncpy(ip, pm->contents[global_seq % pm->size].ip, IP4SIZE);
	*local_seq = pm->contents[global_seq % pm->size].local_seq;

	log_info("[port map]Global Seq = %d, 查询得到客户端ip = %u.%u.%u.%u, seq = %d",
		global_seq, (uint8_t)ip[0], (uint8_t)ip[1], (uint8_t)ip[2], (uint8_t)ip[3], *local_seq);
}

void PortMap_destroy(PortMap * pm)
{
	free(pm->contents);
	free(pm);
}