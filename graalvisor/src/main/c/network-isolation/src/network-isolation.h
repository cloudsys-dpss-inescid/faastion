#ifndef __NETWORKISOLATION_H__
#define __NETWORKISOLATION_H__

void initialize_network_isolation();

void teardown_network_isolation();

int create_network_namespace();

int delete_network_namespace();

#endif