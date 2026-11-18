/* Pure verified remapping of an OG league between manager and resident. */
#ifndef EXPANDED_OG_LEAGUE_BRIDGE_H
#define EXPANDED_OG_LEAGUE_BRIDGE_H
#include <stdint.h>
#define EXP_OG_BRIDGE_MAX_ROWS 64u
static inline int ExpandedOGLeagueBridge_Map(
    const uint16_t *native_ids,unsigned n,const int32_t *native_rows,
    const uint16_t *selected_ids,unsigned selected_count,uint16_t manager_id,
    const int32_t *local_rows,unsigned rows,uint16_t *native_index,
    uint16_t *native_row)
{
    uint8_t seen[EXP_OG_BRIDGE_MAX_ROWS]={0};
    unsigned managers=0u;
    if(!native_ids||!native_rows||!selected_ids||!local_rows||
       !native_index||!native_row||n<2u||n>EXP_OG_BRIDGE_MAX_ROWS||
       rows!=n||selected_count>160u||manager_id==UINT16_MAX)return 0;
    for(unsigned i=0;i<n;i++){
        if(native_ids[i]==UINT16_MAX)return 0;
        for(unsigned j=0;j<i;j++)if(native_ids[i]==native_ids[j])return 0;
    }
    for(unsigned i=0;i<n;i++){
        const int32_t index=local_rows[i];
        uint16_t id;
        if(index==-1){id=manager_id;++managers;}
        else if(index>=0&&(unsigned)index<selected_count)
            id=selected_ids[index];
        else return 0;
        unsigned found=0u,at=0u,row_found=0u,row_at=0u;
        for(unsigned j=0;j<n;j++)if(native_ids[j]==id){++found;at=j;}
        if(found!=1u||seen[at])return 0;
        seen[at]=1u;
        for(unsigned j=0;j<n;j++)
            if(native_rows[j]==(int32_t)at){++row_found;row_at=j;}
        if(row_found!=1u)return 0;
        native_index[i]=(uint16_t)at;native_row[i]=(uint16_t)row_at;
    }
    return managers==1u;
}
typedef struct {
    uint16_t played,wins,draws,losses,for_goals,against_goals,points;
} ExpandedOGLeagueTotals;
static inline int ExpandedOGLeagueBridge_CheckStep(
    const ExpandedOGLeagueTotals *before,const ExpandedOGLeagueTotals *after,
    const uint16_t *native_index,unsigned n,unsigned home,unsigned away,
    unsigned gh,unsigned ga,unsigned points_win)
{
    if(!before||!after||!native_index||n<2u||n>EXP_OG_BRIDGE_MAX_ROWS||
       home>=n||away>=n||home==away||gh>255u||ga>255u||
       points_win>10u)return 0;
    for(unsigned i=0;i<n;i++){
        if(native_index[i]>=n)return 0;
        ExpandedOGLeagueTotals expected=before[native_index[i]];
        if(native_index[i]==home||native_index[i]==away){
            const int is_home=native_index[i]==home;
            const unsigned scored=is_home?gh:ga,conceded=is_home?ga:gh;
            if(expected.played==UINT16_MAX||
               expected.for_goals>UINT16_MAX-scored||
               expected.against_goals>UINT16_MAX-conceded)return 0;
            ++expected.played;
            expected.for_goals+=(uint16_t)scored;
            expected.against_goals+=(uint16_t)conceded;
            if(scored>conceded){
                if(expected.wins==UINT16_MAX||
                   expected.points>UINT16_MAX-points_win)return 0;
                ++expected.wins;expected.points+=(uint16_t)points_win;
            }else if(scored<conceded){
                if(expected.losses==UINT16_MAX)return 0;
                ++expected.losses;
            }else{
                if(expected.draws==UINT16_MAX||
                   expected.points==UINT16_MAX)return 0;
                ++expected.draws;++expected.points;
            }
        }
        const ExpandedOGLeagueTotals *got=&after[i];
        if(got->played!=expected.played||got->wins!=expected.wins||
           got->draws!=expected.draws||got->losses!=expected.losses||
           got->for_goals!=expected.for_goals||
           got->against_goals!=expected.against_goals||
           got->points!=expected.points)return 0;
    }
    return 1;
}
#endif
