#!/usr/bin/env python3
"""Build data_json/worlds/2627/rankings/fifa_nations.json (WORLD 2627 only).

Inputs (read-only, approved research data, never modified):
  docs/competition_research/WORLD/FIFA_WC2026_PRESTART_RANKING_INPUT.json  projection (1 Jul 2026)
  docs/competition_research/WORLD/FIFA_RANKING_NATIONAL_ID_MAP_2627.json   identities
  data_json/worlds/2627/teams/<REGION>.json                                game national DB
Flags: 12x8 native pixel art, one colour letter per pixel, drawn from the
compact specs below (no emoji, no generic placeholder). A PNG preview sheet is
written to development_assets/fifa_flags/ for visual review.
Usage: python tools/fifa_ranking/build_fifa_nations.py [--check]
"""
import json, math, os, struct, sys, zlib

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
W, H = 12, 8
COL = {  # letter -> RGB (rendered with the nearest OG menu palette entry)
    'R': (200, 16, 46), 'W': (255, 255, 255), 'B': (0, 36, 125), 'U': (0, 87, 183),
    'L': (100, 170, 230), 'G': (0, 130, 60), 'Y': (250, 205, 0), 'K': (0, 0, 0),
    'O': (240, 120, 0), 'M': (130, 20, 45), 'N': (140, 90, 40), 'S': (160, 160, 160),
    'T': (60, 60, 60),  # transparent: the OG panel shows through (Nepal only)
}
UKC = ["WBRRBW", "RRRRRR", "BWRRWB", "WBRRBW"]  # 6x4 Union Jack canton

FLAGS = {
 'ARG': 'h:LWL;rect:5,3,2,2,Y', 'ESP': 'hw:R2Y4R2;rect:3,3,1,2,R', 'FRA': 'v:BWR',
 'ENG': 'bg:W;cross:R', 'POR': 'vw:G5R7;rect:4,3,2,2,Y', 'BRA': 'bg:G;diamond:Y;circ:6,4,1.6,B',
 'MAR': 'bg:R;rect:5,3,2,2,G', 'NED': 'h:RWU', 'BEL': 'v:KYR', 'GER': 'h:KRY',
 'CRO': 'h:RWU;rect:5,2,2,3,W;dot:5,2,R;dot:6,3,R;dot:5,4,R', 'ITA': 'v:GWR',
 'COL': 'hw:Y4U2R2', 'MEX': 'v:GWR;rect:5,3,2,2,N', 'SEN': 'v:GYR;rect:5,3,2,2,G',
 'URU': 'h:WLWLWLWL;rect:0,0,4,4,W;rect:1,1,2,2,Y', 'USA': 'h:RWRWRWRW;rect:0,0,5,4,B;dot:1,1,W;dot:3,1,W;dot:2,2,W',
 'JPN': 'bg:W;circ:5.5,3.5,2.2,R', 'SUI': 'bg:R;rect:5,2,2,4,W;rect:4,3,4,2,W',
 'IRN': 'h:GWR;rect:5,3,2,2,R', 'DEN': 'bg:R;nordic:W',
 'TUR': 'bg:R;circ:4.5,3.5,2.4,W;circ:5.4,3.5,2,R;rect:7,3,2,2,W', 'ECU': 'hw:Y4U2R2;rect:5,3,2,2,N',
 'AUT': 'h:RWR', 'KOR': 'bg:W;circh:5.5,3.5,2.1,R,U;dot:1,1,K;dot:10,1,K;dot:1,6,K;dot:10,6,K',
 'NGA': 'v:GWG', 'AUS': 'bg:B;ukc;dot:9,2,W;dot:8,5,W;dot:10,5,W;dot:3,6,W',
 'ALG': 'v:GW;circ:5.5,3.5,2,R;circ:6.2,3.5,1.6,W;dot:7,3,R', 'EGY': 'h:RWK;rect:5,3,2,2,Y',
 'CAN': 'vw:R3W6R3;rect:5,2,2,4,R;rect:4,3,4,2,R', 'NOR': 'bg:R;nordic3:W;nordic:B',
 'UKR': 'h:UY', 'CIV': 'v:OWG', 'PAN': 'rect:0,0,6,4,W;rect:6,0,6,4,R;rect:0,4,6,4,B;rect:6,4,6,4,W;dot:3,2,B;dot:8,5,R',
 'RUS': 'h:WUR', 'POL': 'h:WR', 'WAL': 'h:WG;rect:4,2,4,4,R;dot:3,3,R;dot:8,2,R',
 'SWE': 'bg:U;nordic:Y', 'HUN': 'h:RWG', 'CZE': 'h:WR;tri:U,6', 'PAR': 'h:RWU;rect:5,3,2,2,Y',
 'SCO': 'bg:U;saltire:W', 'SRB': 'h:RUW;rect:2,2,2,3,R;dot:2,3,W', 'CMR': 'v:GRY;rect:5,3,2,2,Y',
 'TUN': 'bg:R;circ:5.5,3.5,2.5,W;circ:5.9,3.5,1.6,R;dot:6,3,W', 'COD': 'bg:L;diagw:Y;diag:R;rect:1,1,2,2,Y',
 'SVK': 'h:WUR;rect:2,2,3,4,R;dot:3,3,W', 'GRE': 'h:UWUWUWUW;rect:0,0,5,4,U;rect:2,0,1,4,W;rect:0,1,5,1,W',
 'VEN': 'h:YUR;dot:3,3,W;dot:5,2,W;dot:7,2,W;dot:9,3,W', 'UZB': 'hw:L3W2G3;rect:0,2,12,1,R;rect:0,5,12,1,R;dot:1,1,W',
 'CHI': 'hw:W4R4;rect:0,0,4,4,B;dot:2,2,W', 'PER': 'v:RWR', 'CRC': 'hw:B1W1R4W1B1',
 'ROU': 'v:BYR', 'MLI': 'v:GYR', 'QAT': 'vw:W3M9;dot:3,1,M;dot:3,3,M;dot:3,5,M', 'IRQ': 'h:RWK;rect:4,3,4,2,G',
 'IRL': 'v:GWO', 'SVN': 'h:WUR;rect:2,1,3,3,U;dot:3,2,W', 'RSA': 'hw:R3G2B3;rect:0,2,12,1,W;rect:0,5,12,1,W;rect:0,3,12,2,G;tri:Y,5;tri:K,4',
 'KSA': 'bg:G;rect:3,3,6,1,W;rect:3,5,6,1,W', 'BFA': 'h:RG;rect:5,3,2,2,Y', 'JOR': 'h:KWG;tri:R,5;dot:1,4,W',
 'BIH': 'bg:B;rtri:Y;dot:3,0,W;dot:5,2,W;dot:7,4,W;dot:9,6,W', 'HON': 'h:UWU;dot:5,3,U;dot:6,4,U;dot:4,4,U;dot:7,3,U',
 'ALB': 'bg:R;rect:4,2,4,4,K;dot:3,2,K;dot:8,2,K', 'CPV': 'hw:B4W1R1W1B1;circ:4,5,1.6,Y;circ:4,5,0.9,B',
 'UAE': 'hw:G3W2K3;rect:0,0,3,8,R', 'MKD': 'bg:R;saltire:Y;cross:Y;circ:5.5,3.5,1.6,Y',
 'NIR': 'bg:W;cross:R;rect:5,3,2,2,W;dot:5,3,R', 'JAM': 'bg:G;lrtri:K;saltire:Y', 'GEO': 'bg:W;cross:R;dot:2,1,R;dot:9,1,R;dot:2,6,R;dot:9,6,R',
 'GHA': 'h:RYG;rect:5,3,2,2,K', 'ISL': 'bg:U;nordic3:W;nordic:R', 'FIN': 'bg:W;nordic:U',
 'ISR': 'hw:W1U1W4U1W1;rect:5,3,2,2,U;dot:5,3,W', 'BOL': 'h:RYG', 'KOS': 'bg:U;rect:4,3,4,3,Y;dot:3,1,W;dot:5,1,W;dot:7,1,W;dot:9,1,W',
 'OMA': 'hw:W3R2G3;rect:0,0,3,8,R;dot:1,1,W', 'MNE': 'bg:Y;rect:1,1,10,6,R;rect:4,2,4,4,Y', 'GUI': 'v:RYG',
 'CUW': 'bg:U;rect:0,5,12,1,Y;dot:2,1,W;dot:3,2,W', 'HAI': 'h:UR;rect:5,3,2,2,W', 'SYR': 'h:RWK;dot:4,3,G;dot:7,3,G',
 'NZL': 'bg:B;ukc;dot:9,2,R;dot:8,4,R;dot:10,4,R;dot:9,6,R', 'GAB': 'h:GYU', 'BUL': 'h:WGR', 'ANG': 'h:RK;rect:5,3,2,2,Y',
 'UGA': 'hw:K1Y1R2K1Y1R2;circ:5.5,3.5,1.7,W', 'ZAM': 'bg:G;rect:8,3,1,5,R;rect:9,3,1,5,K;rect:10,3,1,5,O;rect:8,1,3,1,O',
 'CHN': 'bg:R;rect:1,1,2,2,Y;dot:4,1,Y;dot:4,3,Y', 'BHR': 'vw:W3R9;dot:3,1,R;dot:3,3,R;dot:3,5,R',
 'BEN': 'hw:Y4R4;rect:0,0,5,8,G', 'THA': 'hw:R1W1B4W1R1', 'PLE': 'h:KWG;tri:R,5',
 'BLR': 'hw:R5G3;rect:0,0,2,8,W;dot:1,2,R;dot:1,5,R', 'GUA': 'v:LWL;rect:5,3,2,2,G', 'LUX': 'h:RWL',
 'VIE': 'bg:R;rect:5,3,2,2,Y;dot:6,2,Y;dot:4,3,Y;dot:7,3,Y', 'SLV': 'h:UWU;rect:5,3,2,2,Y', 'TJK': 'hw:R2W4G2;rect:5,3,2,1,Y',
 'TRI': 'bg:R;diagw:W;diag:K', 'MOZ': 'h:GKY;rect:0,2,12,1,W;rect:0,5,12,1,W;tri:R,5;dot:1,4,Y',
 'MAD': 'hw:R4G4;rect:0,0,4,8,W', 'EQG': 'h:GWR;tri:L,4;rect:5,3,2,2,G', 'KGZ': 'bg:R;circ:5.5,3.5,2.2,Y;circ:5.5,3.5,1.2,R',
 'ARM': 'h:RUO', 'COM': 'hw:Y2W2R2U2;tri:G,5;dot:1,4,W', 'KEN': 'hw:K2W1R2W1G2;rect:5,2,2,4,K;dot:5,3,R;dot:6,4,R',
 'LBY': 'hw:R2K4G2;rect:5,3,2,2,W;dot:6,3,K', 'KAZ': 'bg:L;circ:5.5,3.5,1.8,Y;rect:0,0,1,8,Y',
 'TAN': 'bg:L;ultri:G;diagw:Y;diag:K', 'MTN': 'hw:R1G6R1;rect:5,3,2,2,Y;dot:4,2,Y;dot:7,2,Y',
 'NIG': 'h:OWG;circ:5.5,3.5,1,O', 'LBN': 'hw:R2W4R2;rect:5,3,2,2,G;dot:5,2,G;dot:6,2,G', 'GAM': 'hw:R3U2G3;rect:0,2,12,1,W;rect:0,5,12,1,W',
 'SDN': 'h:RWK;tri:G,5', 'IDN': 'h:RW', 'TOG': 'h:GYGYG;rect:0,0,5,4,R;dot:2,2,W',
 'PRK': 'hw:U1R6U1;circ:3.5,3.5,1.7,W;dot:3,3,R', 'NAM': 'bg:G;ultri:U;diagw:W;diag:R;dot:2,1,Y',
 'SLE': 'h:GWL', 'FRO': 'bg:W;nordic3:U;nordic:R', 'CYP': 'bg:W;rect:4,3,4,2,O;dot:5,5,G;dot:6,5,G',
 'SUR': 'hw:G2W1R2W1G2;rect:5,3,2,2,Y', 'AZE': 'h:LRG;rect:5,3,2,2,W;dot:6,3,R', 'EST': 'h:LKW',
 'RWA': 'hw:L4Y2G2;rect:9,1,2,2,Y', 'MWI': 'h:KRG;rect:4,1,4,1,R', 'ZIM': 'hw:G1Y1R1K2R1Y1G1;tri:W,5;dot:1,4,R',
 'NCA': 'h:UWU;rect:5,3,2,2,L', 'GNB': 'hw:Y4G4;rect:0,0,4,8,R;dot:2,4,K', 'KUW': 'h:GWR;tri:K,4',
 'CGO': 'bg:R;ultri:G;diag:Y', 'PHI': 'hw:B4R4;tri:W,6;dot:2,4,Y', 'MAS': 'h:RWRWRWRW;rect:0,0,6,4,B;dot:2,2,Y;dot:3,1,Y',
 'LVA': 'hw:M3W2M3', 'IND': 'h:OWG;rect:5,3,2,2,B', 'CTA': 'h:BWGY;rect:5,0,2,8,R;dot:1,0,Y',
 'LBR': 'h:RWRWRWRW;rect:0,0,4,4,B;dot:2,2,W', 'TKM': 'bg:G;rect:1,0,3,8,M;rect:5,1,1,2,W;dot:7,1,W',
 'BDI': 'bg:R;lrtri:G;saltire:W;circ:5.5,3.5,1.9,W;dot:5,3,R;dot:6,4,R', 'ETH': 'h:GYR;circ:5.5,3.5,1.9,U;dot:5,3,Y;dot:6,4,Y',
 'DOM': 'bg:W;rect:0,0,5,3,U;rect:7,0,5,3,R;rect:0,5,5,3,R;rect:7,5,5,3,U;rect:5,3,2,2,G', 'YEM': 'h:RWK',
 'LES': 'hw:U2W4G2;rect:5,3,2,2,K', 'BOT': 'hw:L3K2L3;rect:0,2,12,1,W;rect:0,5,12,1,W', 'SGP': 'h:RW;rect:1,1,1,2,W;dot:3,1,W;dot:3,2,W',
 'LTU': 'h:YGR', 'GUY': 'bg:G;tri:W,12;tri:Y,11;tri:K,6;tri:R,5', 'NCL': 'h:URG;circ:3.5,3.5,2.2,Y;rect:3,2,1,4,K',
 'SKN': 'bg:R;ultri:G;diagw:Y;diag:K;dot:4,5,W;dot:7,2,W', 'SOL': 'bg:G;ultri:B;diag:Y;dot:1,1,W;dot:3,1,W;dot:2,2,W',
 'PUR': 'h:RWRWR;tri:L,5;dot:1,4,W', 'FIJ': 'bg:L;ukc;rect:8,2,3,4,W;rect:8,2,3,1,R', 'HKG': 'bg:R;circ:5.5,3.5,2,W;dot:5,3,R;dot:6,4,R',
 'TAH': 'hw:R2W4R2;rect:5,3,2,2,Y;dot:5,4,R', 'MYA': 'h:YGR;rect:5,3,2,2,W;dot:5,2,W;dot:6,2,W;dot:4,3,W;dot:7,3,W',
 'MDA': 'v:UYR;rect:5,3,2,2,N', 'VAN': 'hw:R3K2G3;tri:Y,6;tri:K,5;dot:1,4,Y', 'MLT': 'v:WR;dot:1,1,S',
 'ATG': 'bg:R;vtri:W;vtrib:U,0,5;vtrib:K,0,3;rect:5,2,2,1,Y', 'GRN': 'bg:R;rect:1,1,10,6,G;rect:3,1,6,1,Y;rect:3,6,6,1,Y;rect:5,2,2,4,Y;circ:5.5,3.5,1,R',
 'CUB': 'h:UWUWU;tri:R,5;dot:1,4,W', 'SWZ': 'hw:U1Y1R4Y1U1;rect:4,3,4,2,K;rect:6,3,2,2,W',
 'LCA': 'bg:L;utri:W,8,7;utri:K,6,6;utri:Y,8,3', 'BER': 'bg:R;ukc;rect:8,3,2,3,W;dot:8,5,R', 'PNG': 'bg:R;lltri:K;dot:2,5,W;dot:3,6,W;dot:1,6,W;rect:7,1,2,2,Y',
 'SSD': 'hw:K2W1R2W1G2;tri:U,5;dot:1,4,Y', 'VIN': 'vw:U3Y6G3;dot:5,3,G;dot:6,4,G;dot:7,3,G;dot:4,4,G',
 'AFG': 'v:KRG;rect:5,3,2,2,W', 'AND': 'v:UYR;rect:5,3,2,2,N', 'MDV': 'bg:R;rect:2,2,8,4,G;rect:5,3,2,2,W;dot:6,3,G',
 'TPE': 'bg:W;circ:5.5,3.2,2.5,R;circ:5.5,3.2,1.5,U;circ:5.5,3.2,0.8,W;rect:4,7,4,1,U',
 'CAM': 'hw:U2R4U2;rect:4,3,4,2,W;dot:4,3,R;dot:7,3,R', 'MSR': 'bg:B;ukc;rect:8,2,2,4,G;dot:8,2,L',
 'NEP': 'nepal', 'MRI': 'h:RUYG', 'BRB': 'v:UYU;rect:5,2,2,4,K;dot:4,2,K;dot:7,2,K', 'BLZ': 'hw:R1U6R1;circ:5.5,3.5,2,W;dot:5,3,G;dot:6,4,G',
 'BAN': 'bg:G;circ:5,3.5,2.3,R', 'DMA': 'bg:G;cross3:Y;cross:K;circ:5.5,3.5,1.9,R', 'CHA': 'v:UYR',
 'ERI': 'hw:G4U4;tri:R,12;dot:3,4,Y', 'LAO': 'hw:R2U4R2;circ:5.5,3.5,1.6,W', 'COK': 'bg:B;ukc;dot:8,3,W;dot:10,3,W;dot:9,2,W;dot:9,5,W;dot:8,4,W;dot:10,4,W',
 'SRI': 'bg:Y;rect:1,1,2,6,G;rect:3,1,1,6,O;rect:5,1,6,6,M;dot:8,4,Y;dot:7,3,Y', 'SAM': 'bg:R;rect:0,0,6,4,B;dot:2,1,W;dot:3,2,W;dot:1,2,W',
 'ARU': 'bg:L;rect:0,5,12,1,Y;rect:0,7,12,1,Y;dot:1,1,R', 'MNG': 'v:RUR;rect:1,2,2,4,Y', 'ASA': 'bg:U;ftri:R,12;ftri:W,11;rect:7,3,2,2,N',
 'BHU': 'bg:O;ultri:Y;rect:4,3,4,2,W', 'MAC': 'bg:G;rect:4,3,4,2,W;dot:6,2,Y;dot:5,1,Y;dot:7,1,Y', 'BRU': 'bg:Y;bdiag:W,-1;bdiag:K,1;rect:5,3,2,2,R',
 'STP': 'hw:G3Y2G3;tri:R,4;dot:6,4,K;dot:9,4,K', 'DJI': 'hw:L4G4;tri:W,6;dot:2,4,R', 'CAY': 'bg:B;ukc;rect:8,3,2,3,W;dot:8,3,R',
 'PAK': 'bg:G;rect:0,0,3,8,W;circ:7,4,2,W;circ:7.6,3.6,1.7,G;dot:8,2,W', 'SOM': 'bg:L;rect:5,3,2,2,W;dot:5,2,W;dot:4,4,W;dot:7,4,W',
 'TGA': 'bg:R;rect:0,0,5,4,W;rect:1,2,3,1,R;rect:2,1,1,3,R', 'TLS': 'bg:R;tri:Y,7;tri:K,5;dot:1,4,W', 'GIB': 'hw:W5R3;rect:4,1,4,3,R;dot:5,4,Y',
 'GUM': 'bg:B;rect:0,0,12,1,R;rect:0,7,12,1,R;rect:0,0,1,8,R;rect:11,0,1,8,R;circ:5.5,3.5,1.8,L;rect:5,4,2,1,N',
 'SEY': 'rays:UYRWG', 'TCA': 'bg:B;ukc;rect:8,2,2,4,Y;dot:8,3,O', 'LIE': 'hw:U4R4;dot:2,1,Y;dot:3,1,Y;dot:2,2,Y;dot:3,2,Y',
 'BAH': 'hw:L3Y2L3;tri:K,5', 'VIR': 'bg:W;rect:4,2,4,4,Y;rect:5,3,2,2,U', 'VGB': 'bg:B;ukc;rect:8,3,2,3,G;dot:8,3,W',
 'AIA': 'bg:B;ukc;rect:8,3,2,3,W;dot:9,4,O;dot:8,5,L;dot:9,5,L', 'SMR': 'h:WL;rect:5,3,2,2,Y',
}


def stripes(n_rows, weights):
    """Split n_rows into len(weights) bands; return list of (start,end)."""
    tot = sum(weights); out = []; acc = 0
    for w in weights:
        a = round(acc * n_rows / tot); acc += w; b = round(acc * n_rows / tot); out.append((a, b))
    return out


def weighted(arg):
    cols, ws, i = [], [], 0
    while i < len(arg):
        c = arg[i]; i += 1; j = i
        while j < len(arg) and arg[j].isdigit(): j += 1
        cols.append(c); ws.append(int(arg[i:j])); i = j
    return cols, ws


def render(spec):
    px = [['W'] * W for _ in range(H)]
    def put(x, y, c):
        if 0 <= x < W and 0 <= y < H: px[y][x] = c
    def rect(x, y, w, h, c):
        for yy in range(y, y + h):
            for xx in range(x, x + w): put(xx, yy, c)
    if spec == 'nepal':  # double pennant, T = transparent (panel background)
        rect(0, 0, W, H, 'T')
        for y in range(H):
            top = y < 4
            reach = (8 - (y if top else y - 4) * 2) if top else (9 - (y - 4) * 2)
            for x in range(max(2, reach)): put(x, y, 'R')
            put(0, y, 'U')
        put(2, 2, 'W'); put(2, 6, 'W')
        return px
    for cmd in spec.split(';'):
        if not cmd: continue
        op, _, arg = cmd.partition(':')
        a = arg.split(',') if arg else []
        if op == 'bg': rect(0, 0, W, H, a[0])
        elif op == 'h':
            for (s, e), c in zip(stripes(H, [1] * len(arg)), arg): rect(0, s, W, e - s, c)
        elif op == 'v':
            for (s, e), c in zip(stripes(W, [1] * len(arg)), arg): rect(s, 0, e - s, H, c)
        elif op == 'hw':
            cs, ws = weighted(arg)
            for (s, e), c in zip(stripes(H, ws), cs): rect(0, s, W, e - s, c)
        elif op == 'vw':
            cs, ws = weighted(arg)
            for (s, e), c in zip(stripes(W, ws), cs): rect(s, 0, e - s, H, c)
        elif op == 'rect': rect(int(a[0]), int(a[1]), int(a[2]), int(a[3]), a[4])
        elif op == 'dot': put(int(a[0]), int(a[1]), a[2])
        elif op == 'cross': rect(5, 0, 2, H, a[0]); rect(0, 3, W, 2, a[0])
        elif op == 'cross3': rect(4, 0, 4, H, a[0]); rect(0, 2, W, 4, a[0])
        elif op == 'nordic': rect(3, 0, 2, H, a[0]); rect(0, 3, W, 2, a[0])
        elif op == 'nordic3': rect(2, 0, 4, H, a[0]); rect(0, 2, W, 4, a[0])
        elif op == 'saltire':
            for x in range(W):
                y = round(x * (H - 1) / (W - 1)); put(x, y, a[0]); put(x, H - 1 - y, a[0])
        elif op in ('diag', 'diagw'):  # band lower-left -> upper-right
            half = 1.6 if op == 'diag' else 2.6
            for y in range(H):
                for x in range(W):
                    d = (x + 0.5) / W + (y + 0.5) / H - 1.0
                    if abs(d) * 4 < half / 2: put(x, y, a[0])
        elif op == 'bdiag':  # band upper-left -> lower-right, offset
            off = int(a[1])
            for y in range(H):
                for x in range(W):
                    if abs(round(x * H / W) - y - off) <= 0: put(x, y, a[0])
        elif op == 'ultri':  # upper-left triangle of the diagonal
            for y in range(H):
                for x in range(W):
                    if (x + 0.5) / W + (y + 0.5) / H < 1.0: put(x, y, a[0])
        elif op == 'lltri':  # lower-left triangle (top-left -> bottom-right split)
            for y in range(H):
                for x in range(W):
                    if (y + 0.5) / H > (x + 0.5) / W: put(x, y, a[0])
        elif op == 'rtri':  # right triangle, vertical side on the fly
            for y in range(H):
                for x in range(W):
                    if x >= 3 and (x - 3) * H >= y * (W - 4): put(x, y, a[0])
        elif op in ('tri', 'ftri'):  # hoist (or fly) triangle, depth w
            w = int(a[1]) if len(a) > 1 else 5
            for y in range(H):
                d = abs((y + 0.5) - H / 2) / (H / 2)
                n = round(w * (1 - d))
                for x in range(n): put(x if op == 'tri' else W - 1 - x, y, a[0])
        elif op == 'lrtri':
            for y in range(H):
                d = abs((y + 0.5) - H / 2) / (H / 2); n = round(5 * (1 - d))
                for x in range(n): put(x, y, a[0]); put(W - 1 - x, y, a[0])
        elif op in ('vtri', 'vtrib'):  # inverted triangle from the top edge
            y0, y1 = (int(a[1]), int(a[2])) if op == 'vtrib' else (0, H)
            for y in range(y0, y1):
                half = (W / 2) * (1 - y / H)
                for x in range(W):
                    if abs((x + 0.5) - W / 2) < half: put(x, y, a[0])
        elif op == 'utri':  # triangle apex at top, base on the bottom row
            bw, hh = int(a[1]), int(a[2])
            for k in range(hh):
                y = H - 1 - k; half = (bw / 2) * (1 - k / hh)
                for x in range(W):
                    if abs((x + 0.5) - W / 2) < half: put(x, y, a[0])
        elif op == 'diamond':
            for y in range(H):
                for x in range(W):
                    if abs((x + 0.5) - W / 2) / 5.5 + abs((y + 0.5) - H / 2) / 3.5 <= 1: put(x, y, a[0])
        elif op == 'circ':
            cx, cy, r = float(a[0]), float(a[1]), float(a[2])
            for y in range(H):
                for x in range(W):
                    if (x - cx) ** 2 + (y - cy) ** 2 <= r * r: put(x, y, a[3])
        elif op == 'circh':
            cx, cy, r = float(a[0]), float(a[1]), float(a[2])
            for y in range(H):
                for x in range(W):
                    if (x - cx) ** 2 + (y - cy) ** 2 <= r * r: put(x, y, a[3] if y <= cy else a[4])
        elif op == 'ukc':
            for y, row in enumerate(UKC):
                for x, c in enumerate(row): put(x, y, c)
        elif op == 'rays':  # bands radiating from the lower hoist corner
            cols = arg
            for y in range(H):
                for x in range(W):
                    ang = math.atan2((H - 0.5 - y) / H, (x + 0.5) / W)
                    k = min(len(cols) - 1, int(ang / (math.pi / 2) * len(cols)))
                    put(x, y, cols[len(cols) - 1 - k])
        else:
            raise SystemExit('bad flag op %r in %r' % (op, spec))
    return px


def png(path, w, h, rgb_rows):
    raw = b''.join(b'\x00' + bytes(v for p in row for v in p) for row in rgb_rows)
    def chunk(t, d): return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    data = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0)) + \
        chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b'')
    os.makedirs(os.path.dirname(path), exist_ok=True)
    open(path, 'wb').write(data)


def main():
    wd = os.path.join(ROOT, 'docs', 'competition_research', 'WORLD')
    proj = json.load(open(os.path.join(wd, 'FIFA_WC2026_PRESTART_RANKING_INPUT.json'), encoding='utf-8'))
    idm = json.load(open(os.path.join(wd, 'FIFA_RANKING_NATIONAL_ID_MAP_2627.json'), encoding='utf-8'))
    regions = {80: 'EUROPE', 81: 'AFRICA', 82: 'SOUTH_AMERICA', 83: 'NORTH_AMERICA', 84: 'ASIA', 85: 'OCEANIA'}
    teams = {f: json.load(open(os.path.join(ROOT, 'data_json', 'worlds', '2627', 'teams', r + '.json'),
                               encoding='utf-8'))['teams'] for f, r in regions.items()}
    ids = {r['code']: r for r in idm['records']}
    assert len(ids) == 211 and len(proj['projection']) == 211, 'expected 211 nations'
    missing = sorted(set(ids) - set(FLAGS))
    assert not missing, 'flags missing: %s' % missing
    rows, seen = [], set()
    for p in sorted(proj['projection'], key=lambda q: -q['pointsApprox']):
        c = p['code']; r = ids[c]
        t = teams[r['teamFile']][r['ordinal']]['team']
        assert t['country_number'] == r['teamFile'] and t['team_ordinal'] == r['ordinal'], c
        sc = ((r['teamFile'] << 8) | r['ordinal']) + 1
        assert sc == r['stableClub'] and sc not in seen, c
        seen.add(sc)
        flag = ''.join(''.join(row) for row in render(FLAGS[c]))
        assert len(flag) == W * H and set(flag) <= set(COL), c
        rows.append({'code': c, 'name': t['team_name'], 'teamFile': r['teamFile'], 'ordinal': r['ordinal'],
                     'stableClub': sc, 'points': repr(float(p['pointsApprox'])), 'flag': flag})
    out = {'schema': 1, 'world': '2627', 'asOf': '2026-07-01', 'mode': 'FIFA_SUM_SEED',
           'source': 'FIFA_WC2026_PRESTART_RANKING_INPUT.json projection (user accepted 27/11/2026)',
           'flagSize': [W, H], 'palette': {k: '%02X%02X%02X' % v for k, v in COL.items()},
           'expectedRows': len(rows), 'records': rows}
    text = json.dumps(out, ensure_ascii=True, indent=1) + '\n'
    dst = os.path.join(ROOT, 'data_json', 'worlds', '2627', 'rankings', 'fifa_nations.json')
    if '--check' in sys.argv:
        ok = os.path.exists(dst) and open(dst, encoding='utf-8').read() == text
        print('fifa_nations.json', 'UP TO DATE' if ok else 'STALE'); return 0 if ok else 1
    open(dst, 'w', encoding='utf-8', newline='\n').write(text)
    # preview sheet: 16 columns, 4x scale, rank order
    cols, sc = 16, 4
    nrow = (len(rows) + cols - 1) // cols
    tw, th = (W + 2) * sc, (H + 2) * sc
    img = [[(60, 60, 60)] * (cols * tw) for _ in range(nrow * th)]
    for i, r in enumerate(rows):
        ox, oy = (i % cols) * tw + sc, (i // cols) * th + sc
        for y in range(H):
            for x in range(W):
                c = COL[r['flag'][y * W + x]]
                for yy in range(sc):
                    for xx in range(sc): img[oy + y * sc + yy][ox + x * sc + xx] = c
    png(os.path.join(ROOT, 'development_assets', 'fifa_flags', 'fifa_flags_preview_rank_order.png'),
        cols * tw, nrow * th, img)
    print('wrote', dst, len(rows), 'nations')
    return 0


if __name__ == '__main__':
    sys.exit(main())
