/*
 * aQuantia Corporation Network Driver
 * Copyright (C) 2019 aQuantia Corporation. All rights reserved
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include "atlfwd_args.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libmnl/libmnl.h"

static void print_usage(const char *binname)
{
	fprintf(stderr,
		"Usage: %s [-d devname] [-v] command <command_args>\n\n",
		binname);
	fprintf(stderr, "%s\n", "Options:");
	fprintf(stderr, "\t%s\n",
		"-d devname\t- specify device name to work with");
	fprintf(stderr, "\t%s\n", "-v\t\t- verbose mode");
	fprintf(stderr, "%s\n", "");
	fprintf(stderr, "%s\n", "Supported commands:");
	fprintf(stderr, "\t%s\n",
		"request_ring <flags> <ring_size> <buf_size> <page_order>");
	fprintf(stderr, "\t%s\n", "release_ring <ring_index>");
	fprintf(stderr, "\t%s\n", "enable_ring <ring_index>");
	fprintf(stderr, "\t%s\n", "disable_ring <ring_index>");
	fprintf(stderr, "\t%s\n", "dump_ring <ring_index>");
	fprintf(stderr, "\t%s\n", "request_event <ring_index>");
	fprintf(stderr, "\t%s\n", "release_event <ring_index>");
	fprintf(stderr, "\t%s\n", "enable_event <ring_index>");
	fprintf(stderr, "\t%s\n", "disable_event <ring_index>");
	fprintf(stderr, "%s\n", "");
	fprintf(stderr, "\t%s\n", "force_icmp_tx_via <ring_index>");
	fprintf(stderr, "\t%s\n", "force_tx_via <ring_index>");
	fprintf(stderr, "\t%s\n", "disable_redirections");
	fprintf(stderr, "%s\n", "");
	fprintf(stderr, "\t%s\n", "ring_status [<ring_index>]");
	fprintf(stderr, "\t%s\n", "set_tx_bunch <bunch_size>");
}

static enum atlfwd_nl_command get_command(const char *str)
{
	static const char cmd_req_ring[] = "request_ring";
	static const char cmd_rel_ring[] = "release_ring";
	static const char cmd_enable_ring[] = "enable_ring";
	static const char cmd_disable_ring[] = "disable_ring";
	static const char cmd_req_event[] = "request_event";
	static const char cmd_rel_event[] = "release_event";
	static const char cmd_enable_event[] = "enable_event";
	static const char cmd_disable_event[] = "disable_event";
	static const char cmd_dump_ring[] = "dump_ring";
	static const char cmd_disable_redirections[] = "disable_redirections";
	static const char cmd_force_icmp_tx_via[] = "force_icmp_tx_via";
	static const char cmd_force_tx_via[] = "force_tx_via";
	static const char cmd_ring_status[] = "ring_status";
	static const char cmd_set_tx_bunch[] = "set_tx_bunch";

	if (strncmp(str, cmd_req_ring, MNL_ARRAY_SIZE(cmd_req_ring)) == 0)
		return ATL_FWD_CMD_REQUEST_RING;

	if (strncmp(str, cmd_rel_ring, MNL_ARRAY_SIZE(cmd_rel_ring)) == 0)
		return ATL_FWD_CMD_RELEASE_RING;

	if (strncmp(str, cmd_enable_ring, MNL_ARRAY_SIZE(cmd_enable_ring)) == 0)
		return ATL_FWD_CMD_ENABLE_RING;

	if (strncmp(str, cmd_disable_ring, MNL_ARRAY_SIZE(cmd_disable_ring)) ==
	    0)
		return ATL_FWD_CMD_DISABLE_RING;

	if (strncmp(str, cmd_dump_ring, MNL_ARRAY_SIZE(cmd_dump_ring)) == 0)
		return ATL_FWD_CMD_DUMP_RING;

	if (strncmp(str, cmd_req_event, MNL_ARRAY_SIZE(cmd_req_event)) == 0)
		return ATL_FWD_CMD_REQUEST_EVENT;

	if (strncmp(str, cmd_rel_event, MNL_ARRAY_SIZE(cmd_rel_event)) == 0)
		return ATL_FWD_CMD_RELEASE_EVENT;

	if (strncmp(str, cmd_enable_event, MNL_ARRAY_SIZE(cmd_enable_event)) == 0)
		return ATL_FWD_CMD_ENABLE_EVENT;

	if (strncmp(str, cmd_disable_event, MNL_ARRAY_SIZE(cmd_disable_event)) == 0)
		return ATL_FWD_CMD_DISABLE_EVENT;

	if (strncmp(str, cmd_disable_redirections,
		    MNL_ARRAY_SIZE(cmd_disable_redirections)) == 0)
		return ATL_FWD_CMD_DISABLE_REDIRECTIONS;

	if (strncmp(str, cmd_force_icmp_tx_via,
		    MNL_ARRAY_SIZE(cmd_force_icmp_tx_via)) == 0)
		return ATL_FWD_CMD_FORCE_ICMP_TX_VIA;

	if (strncmp(str, cmd_force_tx_via, MNL_ARRAY_SIZE(cmd_force_tx_via)) ==
	    0)
		return ATL_FWD_CMD_FORCE_TX_VIA;

	if (strncmp(str, cmd_ring_status, MNL_ARRAY_SIZE(cmd_ring_status)) == 0)
		return ATL_FWD_CMD_RING_STATUS;

	if (strncmp(str, cmd_set_tx_bunch, MNL_ARRAY_SIZE(cmd_set_tx_bunch)) == 0)
		return ATL_FWD_CMD_SET_TX_BUNCH;

	return ATL_FWD_CMD_UNSPEC;
}

static const char *get_arg(const int argc, char **argv)
{
	if (argc == optind) {
		/* not enough arguments => exit immediately */
		print_usage(argv[0]);
		exit(EINVAL);
	}

	return argv[optind++];
}

static const char *get_last_arg(const int argc, char **argv)
{
	const char *result = get_arg(argc, argv);

	if (optind != argc) {
		/* too many arguments => exit immediately */
		print_usage(argv[0]);
		exit(EINVAL);
	}

	return result;
}

struct atlfwd_args *parse_args(const int argc, char **argv)
{
	static struct atlfwd_args parsed_args;
	int opt;

	while ((opt = getopt(argc, argv, "d:v")) != -1) {
		switch (opt) {
		case 'd':
			parsed_args.devname = optarg;
			break;
		case 'v':
			parsed_args.verbose = true;
			break;
		default:
			print_usage(argv[0]);
			return NULL;
		}
	}

	parsed_args.cmd = get_command(get_arg(argc, argv));

	switch (parsed_args.cmd) {
	case ATL_FWD_CMD_REQUEST_RING:
		parsed_args.flags = (uint32_t)atoi(get_arg(argc, argv));
		parsed_args.ring_size = (uint32_t)atoi(get_arg(argc, argv));
		parsed_args.buf_size = (uint32_t)atoi(get_arg(argc, argv));
		parsed_args.page_order =
			(uint32_t)atoi(get_last_arg(argc, argv));
		break;
	case ATL_FWD_CMD_RING_STATUS:
		if (optind == argc) {
			parsed_args.ring_index = -1;
			break;
		}
		/* fall through */
	case ATL_FWD_CMD_RELEASE_RING:
		/* fall through */
	case ATL_FWD_CMD_ENABLE_RING:
		/* fall through */
	case ATL_FWD_CMD_DISABLE_RING:
		/* fall through */
	case ATL_FWD_CMD_DUMP_RING:
		/* fall through */
	case ATL_FWD_CMD_REQUEST_EVENT:
		/* fall through */
	case ATL_FWD_CMD_RELEASE_EVENT:
		/* fall through */
	case ATL_FWD_CMD_ENABLE_EVENT:
		/* fall through */
	case ATL_FWD_CMD_DISABLE_EVENT:
		/* fall through */
	case ATL_FWD_CMD_FORCE_ICMP_TX_VIA:
		/* fall through */
	case ATL_FWD_CMD_FORCE_TX_VIA:
		parsed_args.ring_index =
			(int32_t)atoi(get_last_arg(argc, argv));
		break;
	case ATL_FWD_CMD_SET_TX_BUNCH:
		parsed_args.tx_bunch =
			(uint32_t)atoi(get_last_arg(argc, argv));
		break;
	case ATL_FWD_CMD_DISABLE_REDIRECTIONS:
		break;
	default:
		print_usage(argv[0]);
		return NULL;
	}

	return &parsed_args;
}
