# Soldier vs Warrior
최신 영상 : 
https://www.youtube.com/watch?v=2V8Miq0muno
# Lag Compensation 사용
- Lag Compensation은 서든어택에서 럽샷이라고 부르는 현상이라 볼 수 있다. 서버 패킷 시간때문에 서버와 모든 클라이언트끼리는 오차가 발생할 수 밖에 없다. Soldier vs Warrior에서도 Projectile이 존재해서 해당 현상이 발생한다.
- 액터에 매 틱마다 위치를 저장하고 Projectile과의 충돌을 판단하기 위해 이를 활용한다.
# Client Side Prediction
- 위 서버, 클라 오차때문에 당연히 클라이언트에서 조금 느릴 수 밖에 없는데.. 이렇게 되면 내가 내 캐릭터로 총을 쏘게 된다면 서버에서 판단 후 동작을 하게 한다면 한 템포 뒤에 반응하기 때문에 매우 플레이어를 답답하게 할거다.
- 따라서 클라이언트에서 예상 위치를 미리 계산해서 보여주고 서버에서 패킷을 받으면 해당 위치를 검증해주면 된다.(미사일에 적용)
# Entity Interpolation
- 내 캐릭터가 아닌 다른 캐릭터가 Projectile를 쐈을때 패킷은 매 틱마다 오지않고 당연하게도 듬성듬성 오기때문에 그 사이를 보간을 통해서 부드럽게 맞춰주었다.
# 컨텐츠 개발
- 스킬 시스템(업그레이드 등), 미니맵, 보스 패턴 등 여러 컨턴츠들을 개발하였다.
