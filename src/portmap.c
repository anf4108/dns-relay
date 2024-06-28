#include <portmap.h>

PortMap * PortMap_create(int capacity)
{
	PortMap * pm = (PortMap *)malloc(sizeof(PortMap));
	if (pm == NULL)
	{
		log_info("[port map]：malloc Port Map");
		exit(-1);
	}
	pm->ht = pmHashTable_create(capacity);
	pm->currentSeq = 0;
	return pm;

}

bool PortMap_allocSeq(PortMap * pm, char ip[IP4SIZE], SEQTYPE localSeq, SEQTYPE * globalSeq)
{
	if (pm->currentNum == pm->ht->tablesize)
	{
		log_info("[port map]：已满，无法分配新id序号");
		return false;
	}

	pm->currentNum++;
	log_info("[port map]：分配序号 Global Seq = %d\n", pm->currentSeq);
	// 写入pmHashTable
	pmKEYTYPE key;
	key.exseq = pm->currentSeq;
	strncpy(key.ip, ip, IP4SIZE);
	pmHashTable_insert(pm->ht, key, localSeq);
	*globalSeq = pm->currentSeq;

	pm->currentSeq++;
	return true;
}

bool PortMap_querySeq(PortMap * pm, char ip[IP4SIZE], SEQTYPE globalSeq, SEQTYPE * localSeq)
{
	pmKEYTYPE key;
	key.exseq = globalSeq;
	strncpy(key.ip, ip, IP4SIZE);
	bool ans = pmHashTable_find(pm->ht, key, localSeq);
	log_info("[port map]：查询序号 Local Seq = %d\n", localSeq);
	return ans;
}

bool PortMap_remove(PortMap * pm, char ip[IP4SIZE], SEQTYPE globalSeq)
{
	pmKEYTYPE key;
	key.exseq = globalSeq;
	strncpy(key.ip, ip, IP4SIZE);
	pm->currentNum--;

	return pmHashTable_remove(pm->ht, key);
}



void PortMap_destroy(PortMap * pm)
{
	pmHashTable_destroy(pm->ht);
	free(pm);
}
