diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn main(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = 1.0;
    let _skTemp1 = dpdx(_skTemp0);
    (*_stageOut).sk_FragColor.x = f32(_skTemp1);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}