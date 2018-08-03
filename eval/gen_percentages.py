import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
import seaborn as sns
import math

NUM_CACHELINES = 128


def main():
    num_instr = 1536
    is_write = True
    with_idle = True
    idd0_instr = (1 + 1) + (1 + 1) # ACT-WAIT PRE-WAIT
    idd0_clks = 5 + 5 # tRCD + tRP
    compulsory_instr = 1 # END_ISEQ
    compulsory_bubble = 1 # ACT - PRE - END_ISEQ
    compulsory_instr = compulsory_instr + 4 if is_write else compulsory_instr
    compulsory_bubble = compulsory_bubble + 12 if is_write else compulsory_instr
    loop_overhead = compulsory_bubble + idd0_clks

    max_num_ops = (num_instr - compulsory_instr) / 3 # wait-rd-wait

    df = pd.DataFrame()
    for percentage in range(100):
        ratio = percentage / 100.0
        num_ops = max_num_ops
        min_err = 1
        the_num_ops = -1
        the_interval = -1
        the_ratio = -1
        the_period = -1
        the_idle_ratio = -1
        the_idd0_ratio = -1

        if with_idle is True:
            while num_ops > 0:
                if num_ops != 0 and rd_ratio != 0:
                    op_interval = int(round(float(num_ops * 4 - rd_ratio * loop_overhead) / (num_ops * rd_ratio)))
                    op_interval = 4 if op_interval < 4 else op_interval
                    my_ratio = float(num_ops * 4) / (num_ops * op_interval + loop_overhead)
                    period = op_interval * num_ops + loop_overhead
                    idle_ratio = float(period - loop_overhead - 4 * num_ops) / period
                    idd0_ratio = float(idd0_clks) / period
                    err = abs(rd_ratio - my_ratio)
                    if min_err > err:
                        min_err = err
                        the_num_ops = num_ops
                        the_interval = op_interval
                        the_ratio = my_ratio
                        the_period = period
                        the_idle_ratio = idle_ratio
                        the_idd0_ratio = idd0_ratio
                    if err <= 0.001:
                        break
                num_ops -= 1

        if with_idle is False:
            # Fop = 4*Nop / (4*Nop + Tcompulsory)
            # Nop = Fop * Tcompulsory / 4(1-Fop)
            the_interval = 4
            the_num_ops = round(ratio * loop_overhead / (the_interval * (1-ratio)))
            the_period = the_num_ops * the_interval + loop_overhead
            the_ratio = float(4 * num_ops) / the_period
            the_idle_ratio = 0
            the_idd0_ratio = float(idd0_clks) / the_period

        print the_interval, the_num_ops, compulsory_bubble

        df = df.append({
            "num_ops": the_num_ops,
            "interval": the_interval,
            "period": the_period,
            "op_ratio": the_ratio,
            "idle_ratio": the_idle_ratio,
            "idd0_ratio": the_idd0_ratio,
            "compulsory_ratio": 1-(the_ratio+the_idle_ratio+the_idd0_ratio)
        }, ignore_index=True)

    df.to_csv("idleandwrite.csv")
    fig, ax = plt.subplots(1,1,figsize=(16,9))
    # sns.pointplot(
    #     data=df,
    #     x="num_ops",
    #     y="step",
    #     ax=ax
    # )
    #
    # sns.pointplot(
    #     data=df,
    #     x="num_ops",
    #     y="bubble",
    #     ax=ax
    # )

    sns.pointplot(
        data=df,
        x="rd_percentage",
        y="",
        ax=ax
    )

    fig.savefig("./fixedtime.png", bbox_inches="tight")

main()
