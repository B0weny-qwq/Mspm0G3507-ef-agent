# Components 层

`Components/` 放纯算法、状态机和滤波器。这里不能依赖 `BoardDevices/`、`Drivers/`、`Platform/` 或 vendor SDK。

## 当前组件

| 组件 | 职责 | 状态 |
| --- | --- | --- |
| `ef_button` | 按键消抖、单击、双击、长按识别 | `ef_button_t` 保存稳定电平、消抖时间和 click/long 状态 |
| `ef_lowpass` | 整数一阶低通滤波 | `ef_lowpass_i32_t` 保存当前输出值、滤波强度和零点阈值 |
| `ef_component_placeholder` | 占位组件 | 后续可删除或替换 |

## 规则

- 组件接口只接收数据和配置，不读取硬件。
- 状态机必须在头文件或 `doc/` 中说明状态、输入、输出和转换条件。
- 修改算法参数或状态转换时，同步更新使用它的 App/BoardDevices 文档。
