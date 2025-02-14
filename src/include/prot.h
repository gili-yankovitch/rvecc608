#ifndef __PROT_H__
#define __PROT_H__

int flashReadProtect();
int flashReadUnprotect();
void flashPageErase(uint32_t address);
uint32_t flashRead(uint32_t addr);
void flashWrite(uint32_t addr, uint32_t * data);

#endif // __PROT_H__
